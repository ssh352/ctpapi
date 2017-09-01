{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'backtesting',
      'type' : 'static_library',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py . *.h *.cc)',
      ],
      'sources/' :[
        ['exclude','main.cc'],
        ['exclude', 'unittest/.*'],
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
    {
      'target_name' : 'backtesting_unittest',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py unittest *.h *.cc)',
      ],
      'dependencies' : [
        'backtesting',
        '<(DEPTH)/testing/gtest.gyp:gtest',
        '<(DEPTH)/testing/gtest.gyp:gtest_main',
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
        '.',
        '..',
      ],
    },
  ]
}
