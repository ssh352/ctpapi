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
  ]
}
