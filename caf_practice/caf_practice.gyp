{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'caf_practice',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        'main.cc',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
        '<(DEPTH)/third_party/lua/lua.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
    },
    {
      'target_name:' : 'caf_exe',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        'caf_exe.cc',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
     
      },
    
    {
      'target_name:' : 'caf_lib',
      'type' : 'shared_library',
      'variables' : {
      },
      'sources' : [
        'caf_lib.h',
        'caf_lib.cc',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
     
      },
  ]
}
