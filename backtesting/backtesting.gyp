{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'backtesting',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py . *.h *.cc)',
      ],
      'dependencies' : [
        '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      ],
      'defines' : [
        'H5_BUILT_AS_DYNAMIC_LIB',
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '..',
      ],
    },
  ]
}
