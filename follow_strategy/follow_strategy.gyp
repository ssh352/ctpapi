{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
  {
    'target_name' : 'follow_strategy',
    'type' : '<(component)',
    'variables' : {
    },
    'sources' : [
      '<!@(python ../build/glob_files.py . *.h *.cc)',
    ],
    'sources/' :[
      ['exclude', 'unittest/.*'],
    ],
    'dependencies' : [

    ],
    'defines' : [
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
      '<(DEPTH)/third_party/actor-framework/libcaf_core'
    ],
  },
  {
    'target_name' : 'follow_strategy_unittest',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
      '<!@(python ../build/glob_files.py unittest *.h *.cc)',
    ],
    'dependencies' : [
      'follow_strategy',
      '../hpt_core/hpt_core.gyp:hpt_core',
      '<(DEPTH)/testing/gtest.gyp:gtest',
      '<(DEPTH)/testing/gtest.gyp:gtest_main',
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
