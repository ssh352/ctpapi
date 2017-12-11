{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
  {
    'target_name' : 'strategy_unittest',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
      '<!@(python ../build/glob_files.py . *.h *.cc)',
    ],
    'dependencies' : [
      '<(DEPTH)/testing/gtest.gyp:gtest',
      '<(DEPTH)/testing/gtest.gyp:gtest_main',
      '<(DEPTH)/third_party/actor-framework/libcaf_core/libcaf_core.gyp:libcaf_core',
      '../follow_strategy/follow_strategy.gyp:follow_strategy',
      '../hpt_core/hpt_core.gyp:hpt_core',
	  '../bft_core/bft_core.gyp:*',
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
