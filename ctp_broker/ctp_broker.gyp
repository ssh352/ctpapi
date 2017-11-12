{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
  {
    'target_name' : 'ctp_broker',
    'type' : '<(component)',
    'variables' : {
    },
    'sources' : [
      # 'ctp_sub_broker.h',
      # 'ctp_sub_broker.cc',
      'ctp_instrument_broker.h',
      'ctp_instrument_broker.cc',
      'ctp_order_delegate.h',
      'ctp_position_amount.cc',
      'ctp_position_amount.h',
      'ctp_position_effect_flag_strategy.cc',
      'ctp_position_effect_flag_strategy.h',
      'ctp_position_effect_strategy.cc',
      'ctp_position_effect_strategy.h',
    ],
    'dependencies' : [
      # '<(DEPTH)/testing/gtest.gyp:gtest',
      # '<(DEPTH)/testing/gtest.gyp:gtest_main',
      # '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      # '../follow_strategy/follow_strategy.gyp:follow_strategy',
      # '../hpt_core/hpt_core.gyp:hpt_core',
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
    'target_name' : 'ctp_broker_unittest',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
      # 'ctp_sub_broker.h',
      # 'ctp_sub_broker.cc',
      'ctp_sub_account_broker_unittest.cc',
      # 'ctp_instrument_broker_unittest.cc',
      'close_today_aware_unittest.cc',
      'close_today_cost_test.cc',
      'ctp_instrument_broker_test.h',
      'ctp_instrument_broker_test.cc',
      'ctp_sub_account_broker_unittest.cc',
      'generic_position_effect_unittest.cc',
    ],
    'dependencies' : [
      'ctp_broker',
      '<(DEPTH)/testing/gtest.gyp:gtest',
      '<(DEPTH)/testing/gtest.gyp:gtest_main',
      # '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      # '../follow_strategy/follow_strategy.gyp:follow_strategy',
      '../hpt_core/hpt_core.gyp:hpt_core',
    ],
    'defines' : [
    ],
    'includes' : [
    ],
    'include_dirs' : [
      '..',
    ],
  }
  ]
}
