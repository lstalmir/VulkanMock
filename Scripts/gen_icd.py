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
                if command.handle not in cmds.keys():
                    cmds[ command.handle ] = []
                cmds[ command.handle ].append( command )
            except Exception as err:
                if not isinstance( err, AttributeError ):
                    raise err

        self.commands = { handle: ICDGenerator.group_commands_by_extension( commands )
                          for handle, commands
                          in cmds.items() }

    def write_commands( self, out: io.TextIOBase ):
        out.write( '#pragma once\n' )
        out.write( '#define VK_NO_PROTOTYPES\n' )
        out.write( '#include <vulkan/vulkan.h>\n' )
        out.write( '#include <vulkan/vk_icd.h>\n' )
        out.write( '#include <string.h>\n\n' )

        # Define base type for each ICD object
        out.write( 'namespace vkmock\n{\n' )
        out.write( 'struct VkObjectBase {\n' )
        out.write( '  uintptr_t m_LoaderMagic = ICD_LOADER_MAGIC;\n' )
        out.write( '};\n\n' )

        for handle, extensions in self.commands.items():
            if handle is not None:
                out.write( f'struct {handle[2:]}Base\n{{\n' )
                for ext, commands in extensions.items():
                    if ext is not None:
                        out.write( f'#ifdef {ext}\n\n' )
                    for cmd in commands:
                        if cmd.name == 'vkGetInstanceProcAddr' or cmd.name == 'vkGetDeviceProcAddr':
                            continue
                        out.write( f'  virtual {cmd.result} {cmd.name}(\n    ' )
                        out.write( ',\n    '.join( [param.string for param in cmd.params[1:] if not param.vulkansc] ) )
                        out.write( ')' )
                        if handle is not None:
                            out.write( '\n  {' )
                            if cmd.alias is not None:
                                out.write( f' return {cmd.alias}(' )
                                out.write( ', '.join( [param.name for param in cmd.params[1:] if not param.vulkansc] ) )
                                out.write( ');' )
                            elif cmd.result != 'void':
                                out.write( ' return {};' )
                            out.write( ' }\n\n' )
                        else:
                            out.write( ';\n\n' )
                    if ext is not None:
                        out.write( f'#endif // {ext}\n\n' )
                out.write( '};\n\n' )

        out.write( '}\n\n' )

        # Define structures with virtual functions
        for handle, extensions in self.commands.items():
            if handle is not None:
                out.write( f'struct {handle}_T : vkmock::VkObjectBase {{\n' )
                out.write( f'  vkmock::{handle[2:]}Base* m_pImpl = nullptr;\n\n' )
                out.write( f'  {handle}_T( vkmock::{handle[2:]}Base* pImpl ) : m_pImpl( pImpl ) {{}}\n\n' )
                out.write( f'  {handle}_T( const {handle}_T& ) = delete;\n' )
                out.write( f'  {handle}_T& operator=( const {handle}_T& ) = delete;\n\n' )
                out.write( f'  {handle}_T( {handle}_T&& other ) noexcept : m_pImpl( other.m_pImpl ) {{ other.m_pImpl = nullptr; }}\n' )
                out.write( f'  {handle}_T& operator=( {handle}_T&& other ) noexcept {{ m_pImpl = other.m_pImpl; other.m_pImpl = nullptr; return *this; }}\n\n' )
                out.write( f'  ~{handle}_T() {{ delete m_pImpl; }}\n\n' )

            for ext, commands in extensions.items():
                if ext is not None:
                    out.write( f'#ifdef {ext}\n\n' )
                for cmd in commands:
                    if cmd.name == 'vkGetInstanceProcAddr' or cmd.name == 'vkGetDeviceProcAddr':
                        continue
                    out.write( f'  {cmd.result} {cmd.name}(\n    ' )
                    if handle is not None:
                        out.write( ',\n    '.join( [param.string for param in cmd.params[1:] if not param.vulkansc] ) )
                    else:
                        out.write( ',\n    '.join( [param.string for param in cmd.params if not param.vulkansc] ) )
                    out.write( ')' )
                    if handle is not None:
                        out.write( '\n  {\n' )
                        out.write( f'    return m_pImpl->{cmd.name}(' )
                        out.write( ', '.join( [param.name for param in cmd.params[1:] if not param.vulkansc] ) )
                        out.write( ');\n' )
                        out.write( '  }\n\n' )
                    else:
                        out.write( ';\n\n' )
                if ext is not None:
                    out.write( f'#endif // {ext}\n\n' )
            if handle is not None:
                out.write( '};\n\n' )

        # Define entry points to the ICD
        for handle, extensions in self.commands.items():
            if handle is not None:
                for ext, commands in extensions.items():
                    if ext is not None:
                        out.write( f'#ifdef {ext}\n\n' )
                    for cmd in commands:
                        if cmd.name == 'vkGetInstanceProcAddr' or cmd.name == 'vkGetDeviceProcAddr':
                            continue
                        out.write( f'inline {cmd.result} {cmd.name}(\n  ' )
                        out.write( ',\n  '.join( [param.string for param in cmd.params if not param.vulkansc] ) )
                        out.write( ')\n{\n' )
                        out.write( f'  return {cmd.params[0].name}->{cmd.name}(' )
                        out.write( ', '.join( [param.name for param in cmd.params[1:] if not param.vulkansc] ) )
                        out.write( ');\n}\n\n' )
                    if ext is not None:
                        out.write( f'#endif // {ext}\n\n' )

        # vkGetInstanceProcAddr
        out.write( 'inline PFN_vkVoidFunction vkGetDeviceProcAddr(VkDevice device, const char* name);\n\n' )
        out.write( 'inline PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* name)\n{\n' )
        for handle, extensions in self.commands.items():
            for ext, commands in extensions.items():
                if ext is not None:
                    out.write( f'#ifdef {ext}\n' )
                for cmd in commands:
                    out.write( f'  if( !strcmp( "{cmd.name}", name ) ) return reinterpret_cast<PFN_vkVoidFunction>(&{cmd.name});\n' )
                if ext is not None:
                    out.write( f'#endif // {ext}\n' )
        out.write( '  return nullptr;\n}\n\n' )

        # vkGetDeviceProcAddr
        out.write( 'inline PFN_vkVoidFunction vkGetDeviceProcAddr(VkDevice device, const char* name)\n{\n' )
        out.write( '  return vkGetInstanceProcAddr( nullptr, name );\n}\n\n' )

    @staticmethod
    def group_commands_by_extension( commands ):
        extensions = {}
        for cmd in commands:
            if cmd.extension not in extensions.keys():
                extensions[ cmd.extension ] = []
            extensions[ cmd.extension ].append( cmd )
        return extensions

def parse_args():
    parser = argparse.ArgumentParser(description='Generate test ICD')
    parser.add_argument('--vk_xml', type=str, help='Vulkan XML API description')
    parser.add_argument('--output', type=str, help='Output directory')
    return parser.parse_args()

if __name__ == '__main__':
    args = parse_args()
    vk_xml = etree.parse( args.vk_xml )
    icd = ICDGenerator( vk_xml )
    with open( os.path.join( args.output ), 'w' ) as out:
        icd.write_commands( out )
