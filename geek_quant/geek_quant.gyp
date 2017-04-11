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
      'old_follow_strategy.h',
      'old_follow_strategy.cc',
      'order_follow.h',
      'order_follow.cc',
      'caf_defines.h',
      'ctp_trader.h',
      'ctp_trader.cc',
      'ctp_order_dispatcher.h',
      'ctp_order_dispatcher.cc',
      'order_follow_manager.h',
      'order_follow_manager.cc',
      'pending_order_action.h',
      'pending_order_action.cc',
      'caf_defines.h',
      'caf_defines.cc',
      'context.h',
      'context.cc',
      'position_manager.h',
      'position_manager.cc',
      'order_manager.h',
      'order_manager.cc',
      'follow_strategy.h',
      'follow_strategy.cc',
      'follow_strategy_mode.h',
      'follow_strategy_mode.cc',
      'follow_strategy_service.h',
      'follow_strategy_service.cc',
      'instrument_position.h',
      'instrument_position.cc',
      'close_corr_orders_manager.h',
      'close_corr_orders_manager.cc',
      'order.h',
      'order.cc',
      'order_id_mananger.h',
      'order_id_mananger.cc',
    ],
    'dependencies' : [
      #'<(DEPTH)/third_party/actor-framework/libcaf_core/libcaf_core.gyp:libcaf_build_config',
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
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
      'follow_strategy_servcie_unittest.cc',
      'follow_trade_unittest.cc',
      'instrument_follow_unittest.cc',
      'ctp_order_dispatcher_unittest.cc',
      'instrument_follow_sync_orders_unittest.cc',
    ],
    'dependencies' : [
      'follow_trade',
      '<(DEPTH)/testing/gtest.gyp:gtest',
      '<(DEPTH)/testing/gtest.gyp:gtest_main',
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
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
    'target_name' : 'serialize_rtn_order',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
      'serialize_rtn_order.cc',
      'ctp_order_dispatcher.h',
      'ctp_order_dispatcher.cc',
      'serialize_ctp_trader.h',
      'serialize_ctp_trader.cc',
      'caf_defines.h',
      'caf_defines.cc',
    ],
    'dependencies' : [
      '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
    ],
    'include_dirs': [
      '..',
    ],
  },
  ]
}     
