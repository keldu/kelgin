#!/usr/bin/env python3

import sys
import os
import os.path
import glob
import re
import pkgconfig

if sys.version_info < (3,):
    def isbasestring(s):
        return isinstance(s,basestring)
else:
    def isbasestring(s):
        return isinstance(s, (str,bytes))

def add_kel_source_files(self, sources, filetype, lib_env=None, shared=False, target_post=""):

    if isbasestring(filetype):
        dir_path = self.Dir('.').abspath
        filetype = sorted(glob.glob(dir_path+"/"+filetype))

    for path in filetype:
        target_name = re.sub( r'(.*?)(\.cpp|\.c\+\+)', r'\1' + target_post, path )
        if shared:
            target_name+='.os'
            sources.append( self.SharedObject( target=target_name, source=path ) )
        else:
            target_name+='.o'
            sources.append( self.StaticObject( target=target_name, source=path ) )
    pass

freetype_cflags = pkgconfig.cflags('freetype2').split(' ');
for i in range(len(freetype_cflags)):
    freetype_cflags[i] = freetype_cflags[i].removeprefix('-I');

cflags = freetype_cflags + ['#source/', '#']

env=Environment(CPPPATH=cflags,
    CXX='clang++',
    CPPDEFINES=['GIN_UNIX'],
    CXXFLAGS=['-std=c++17','-g','-Wall','-Wextra'],
    LIBS=['kelgin','-ldl','-lfreetype'])
env.__class__.add_source_files = add_kel_source_files

env.sources = []
env.headers = []
env.objects = []

env.render_headers = []

env.common_sources = []
env.common_headers = []

env.daemon_sources = []
env.daemon_headers = []
env.daemon_objects = []

env.example_event_sources = []
env.example_event_objects = []
env.example_teapot_sources = []
env.example_teapot_objects = []
env.example_headers = []

Export('env')
SConscript('source/SConscript')
SConscript('daemon/SConscript')
SConscript('example/SConscript')
SConscript('plugins/SConscript')


# Library build

env_library = env.Clone()

env.objects_shared = []
env_library.add_source_files(env.objects_shared, env.sources, shared=True)
env.library_shared = env_library.SharedLibrary('#bin/kelgin-graphics', [env.objects_shared])

env.objects_static = []
env_library.add_source_files(env.objects_static, env.sources)
env.library_static = env_library.StaticLibrary('#bin/kelgin-graphics', [env.objects_static])

env.Alias('library', [env.library_shared, env.library_static])
env.Alias('library_shared', env.library_shared)
env.Alias('library_static', env.library_static)

env.Default('library')

# Daemon

daemon_env = env.Clone()
daemon_env.Append(LIBS=['pthread'])

daemon_env.add_source_files(env.daemon_objects, env.daemon_sources)
env.daemon_bin = daemon_env.Program('#bin/kelgin-graphicsd', [env.daemon_objects, env.library_shared]);

env.Alias('daemon', env.daemon_bin)

# Examples
example_env = env.Clone()
example_env.Append(LIBS=['pthread'])
example_env.add_source_files(env.example_event_objects, env.example_event_sources)
env.example_event_bin = example_env.Program('#bin/example_event', [env.example_event_objects, env.library_shared]);

example_env.add_source_files(env.example_teapot_objects, env.example_teapot_sources)
env.example_teapot_bin = example_env.Program('#bin/example_teapot', [env.example_teapot_objects, env.library_shared]);

env.Alias('examples', [env.example_event_bin, env.example_teapot_bin])

# Tests
# SConscript('test/SConscript')

# Plugins
env.Alias('plugins', env.plugins)

# Clang format part
env.Append(BUILDERS={'ClangFormat' : Builder(action = 'clang-format --style=file -i $SOURCE')})
env.format_actions = []
def format_iter(env,files):
    for f in files:
        env.format_actions.append(env.AlwaysBuild(env.ClangFormat(target=f+"-clang-format",source=f)))
    pass

format_iter(env,env.sources + env.headers + env.daemon_sources + env.daemon_headers + env.example_event_sources + env.example_teapot_sources + env.example_headers)
env.Alias('format', env.format_actions)
env.Alias('all', ['library','plugins','daemon','examples'])
# env.Alias('test', env.test_program)
env.Install('/usr/local/lib/', [env.library_shared, env.library_static])
env.Install('/usr/local/lib/kelgin-graphics/', [env.plugins])
env.Install('/usr/local/include/kelgin/graphics/', [env.headers])
env.Install('/usr/local/include/kelgin/graphics/render/', [env.render_headers])
env.Install('/usr/local/include/kelgin/graphics/common/', [env.common_headers])

env.Alias('install', '/usr/local/')
