{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'hpt_core',
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
      'target_name' : 'hpt_core_unittest',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        '<!@(python ../build/glob_files.py unittest *.h *.cc)',
      ],
      'dependencies' : [
        'hpt_core',
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
