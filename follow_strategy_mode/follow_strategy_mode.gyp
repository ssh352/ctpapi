{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
  {
    'target_name' : 'follow_trade',
    'type' : '<(component)',
    'variables' : {
    },
    'sources' : [
      '<!@(python ../build/glob_files.py src *.h *.cc)',
    ],
    'dependencies' : [

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
    'target_name' : 'follow_trade_unittest',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
      '<!@(python ../build/glob_files.py unittest *.h *.cc)',
    ],
    'dependencies' : [
      'follow_trade',
      '<(DEPTH)/testing/gtest.gyp:gtest',
      '<(DEPTH)/testing/gtest.gyp:gtest_main',
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
