# Copyright (c) 2024 Lukasz Stalmirski
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

import sys 
import os
import io
import argparse
import xml.etree.ElementTree as etree

class VulkanSpec:
    def __init__( self, vk_xml: etree.ElementTree ):
        self.xml = vk_xml.getroot()
        self.types = self.xml.find( 'types' )
        self.commands = self.xml.find( 'commands' )
        self.extensions = self.xml.find( 'extensions' )
        self.vulkansc = self.xml.find( 'feature[@api="vulkansc"]' )
        self.handles = [ handle.text for handle in self.types.findall( 'type[@category="handle"][type="VK_DEFINE_HANDLE"]/name' ) ]

    def get_command_name( self, cmd: etree.ElementTree ):
        try: # Function definition
            return cmd.find( 'proto/name' ).text
        except: # Alias
            return cmd.get( 'name' )

    def get_command_return_type( self, cmd: etree.ElementTree ):
        return self.get_command_definition( cmd ).find( 'proto/type' ).text

    def get_command_handle( self, cmd: etree.ElementTree ):
        param1 = self.get_command_definition( cmd ).find( 'param[1]/type' ).text
        return param1 if param1 in self.handles else None

    def get_command_extension( self, cmd: etree.ElementTree ):
        try: # Extension function
            command_name = self.get_command_name( cmd )
            return self.extensions.find( f'extension/require/command[@name="{command_name}"]/../..' ).get( 'name' )
        except AttributeError: # Core function
            return None

    def is_vulkansc_command( self, cmd: etree.ElementTree ):
        cmd = self.get_command_definition( cmd )
        # Check if cmd is secure version of a standard functions
        if 'api' in cmd.attrib.keys():
            return cmd.attrib[ 'api' ] == 'vulkansc'
        # Check if cmd is required by vulkansc feature set
        if self.vulkansc is not None:
            command_name = self.get_command_name( cmd )
            return self.vulkansc.find( f'require/command[@name="{command_name}"]' ) is not None
        return False

    def get_command_definition( self, cmd: etree.ElementTree ):
        alias = cmd.get( 'alias' )
        if alias is not None:
            return self.commands.find( f'command/proto[name="{alias}"]/..' )
        return cmd

class VulkanCommandParameter:
    def __init__( self, spec: VulkanSpec, param: etree.Element ):
        self.string = ''.join( param.itertext() )
        self.name = param.find( 'name' ).text
        self.vulkansc = False
        if 'api' in param.attrib.keys():
            self.vulkansc = ( param.attrib[ 'api' ] == 'vulkansc' )

    def __str__( self ):
        return self.string

class VulkanCommand:
    def __init__( self, spec: VulkanSpec, cmd: etree.Element ):
        self.alias = cmd.get( 'alias' )
        self.name = spec.get_command_name( cmd )
        if self.alias is not None:
            cmd = spec.get_command_definition( cmd )
        self.result = spec.get_command_return_type( cmd )
        self.params = [ VulkanCommandParameter( spec, param ) for param in cmd.findall( 'param' ) ]
        self.handle = spec.get_command_handle( cmd )
        self.extension = spec.get_command_extension( cmd )
        self.vulkansc = spec.is_vulkansc_command( cmd )

class ICDGenerator:
    def __init__( self, vk_xml: etree.ElementTree ):
        spec = VulkanSpec( vk_xml )
        cmds = {}

        for cmd in spec.commands.findall( 'command' ):
            try:
                command = VulkanCommand( spec, cmd )
                if command.vulkansc:
                    continue
                if command.name == 'vkGetInstanceProcAddr' or command.name == 'vkGetDeviceProcAddr':
                    continue
                if command.handle not in cmds.keys():
                    cmds[ command.handle ] = []
                cmds[ command.handle ].append( command )
            except Exception as err:
                if not isinstance( err, AttributeError ):
                    raise err

        self.commands = { handle: self.group_commands_by_extension( commands )
                          for handle, commands
                          in cmds.items() }

    def write_icd_base( self, out: io.TextIOBase ):
        out.write( '#pragma once\n' )
        out.write( '#include <vulkan/vulkan.h>\n' )
        out.write( '#include <vulkan/vk_icd.h>\n' )
        out.write( '#include <string.h>\n' )
        out.write( '#include <memory>\n\n' )

        # Define dispatch table for custom mock function pointers
        out.write( 'namespace vkmock\n{\n' )
        out.write( 'struct Functions\n{\n' )
        out.write( '  int SetProcAddr( const char* name, PFN_vkVoidFunction func );\n\n' )
        for handle, extensions in self.commands.items():
            if handle is not None:
                for ext, commands in extensions.items():
                    self.begin_extension_block( out, ext )
                    for cmd in commands:
                        out.write( f'  PFN_{cmd.name} {cmd.name} = nullptr;\n' )
                    self.end_extension_block( out, ext )
        out.write( '};\n\n' )

        # Define base type for each ICD object
        for handle, extensions in self.commands.items():
            if handle is not None:
                out.write( f'struct {handle[2:]}Base\n{{\n' )
                out.write( '  uintptr_t m_LoaderMagic = ICD_LOADER_MAGIC;\n' )
                out.write( f'  Functions* m_pMockFunctions = nullptr;\n\n' )
                out.write( f'  {handle} GetApiHandle() {{ return reinterpret_cast<{handle}>(this); }}\n\n' )
                for ext, commands in extensions.items():
                    self.begin_extension_block( out, ext )
                    for cmd in commands:
                        cmd_method_params = [param for param in cmd.params[1:] if not param.vulkansc]
                        out.write( f'  {cmd.result} {cmd.name}(\n    ' )
                        out.write( ',\n    '.join( [param.string for param in cmd_method_params] ) )
                        out.write( ')\n  {\n' )
                        out.write( f'    if (m_pMockFunctions && m_pMockFunctions->{cmd.name})\n' )
                        out.write( f'      return m_pMockFunctions->{cmd.name}(GetApiHandle()' )
                        if cmd_method_params:
                            out.write( ', ' )
                            out.write( ', '.join( [param.name for param in cmd_method_params] ) )
                        out.write( ');\n' )
                        if cmd.alias is not None:
                            out.write( f'    return {cmd.alias}(' )
                            out.write( ', '.join( [param.name for param in cmd_method_params] ) )
                            out.write( ');\n' )
                        elif cmd.result != 'void':
                            out.write( '    return {};\n' )
                        out.write( '  }\n\n' )
                    self.end_extension_block( out, ext )
                out.write( '};\n\n' )
        out.write( '}\n\n' )

    def write_icd_dispatch( self, out: io.TextIOBase ):
        out.write( '#pragma once\n' )

        # Define entry points to the ICD
        for handle, extensions in self.commands.items():
            if handle is not None:
                for ext, commands in extensions.items():
                    self.begin_extension_block( out, ext )
                    for cmd in commands:
                        cmd_method_params = [param for param in cmd.params[1:] if not param.vulkansc]
                        out.write( f'inline VKAPI_ATTR {cmd.result} VKAPI_CALL {cmd.name}(\n  ' )
                        out.write( ',\n  '.join( [param.string for param in cmd.params if not param.vulkansc] ) )
                        out.write( ')\n{\n' )
                        out.write( f'  return {cmd.params[0].name}->{cmd.name}(' )
                        out.write( ', '.join( [param.name for param in cmd_method_params] ) )
                        out.write( ');\n}\n\n' )
                    self.end_extension_block( out, ext )

        # vkGetInstanceProcAddr
        out.write( 'inline VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* name)\n{\n' )
        for handle, extensions in self.commands.items():
            for ext, commands in extensions.items():
                self.begin_extension_block( out, ext )
                for cmd in commands:
                    out.write( f'  if(!strcmp("{cmd.name}", name)) return reinterpret_cast<PFN_vkVoidFunction>(&{cmd.name});\n' )
                self.end_extension_block( out, ext )
        out.write( '  return nullptr;\n}\n\n' )

        # vkmock::Functions::SetProcAddr
        out.write( 'namespace vkmock\n{\n' )
        out.write( '  inline int Functions::SetProcAddr(const char* name, PFN_vkVoidFunction func)\n  {\n' )
        for handle, extensions in self.commands.items():
            if handle is not None:
                for ext, commands in extensions.items():
                    self.begin_extension_block( out, ext )
                    for cmd in commands:
                        out.write( f'    if(!strcmp("{cmd.name}", name)) return ({cmd.name} = reinterpret_cast<PFN_{cmd.name}>(func)), 0;\n' )
                    self.end_extension_block( out, ext )
        out.write( '    return -1;\n  }\n}\n' )

    def group_commands_by_extension( self, commands ):
        extensions = {}
        for cmd in commands:
            if cmd.extension not in extensions.keys():
                extensions[ cmd.extension ] = []
            extensions[ cmd.extension ].append( cmd )
        return extensions

    def begin_extension_block( self, out: io.TextIOBase, ext: str ):
        if ext is not None:
            out.write( f'#ifdef {ext}\n' )

    def end_extension_block( self, out: io.TextIOBase, ext: str ):
        if ext is not None:
            out.write( f'#endif // {ext}\n' )

def parse_args():
    parser = argparse.ArgumentParser( description='Generate test ICD' )
    parser.add_argument( '--vk_xml', type=str, help='Vulkan XML API description' )
    parser.add_argument( '--output', type=str, help='Output directory' )
    return parser.parse_args()

if __name__ == '__main__':
    args = parse_args()
    vk_xml = etree.parse( args.vk_xml )
    icd = ICDGenerator( vk_xml )
    with open( os.path.join( args.output, 'vk_mock_icd_base.h' ), 'w' ) as out:
        icd.write_icd_base( out )
    with open( os.path.join( args.output, 'vk_mock_icd_dispatch.h' ), 'w' ) as out:
        icd.write_icd_dispatch( out )
