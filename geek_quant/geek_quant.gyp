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
      'instrument_follow.h',
      'instrument_follow.cc',
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
    'target_name' : 'follow_trade_server',
    'type' : 'executable',
    'variables' : {
    },
    'sources' : [
      'main.cc',
      'cta_trade_actor.h',
      'cta_trade_actor.cc',
      'follow_trade_actor.h',
      'follow_trade_actor.cc',
    ],
    'dependencies' : [
      '<(DEPTH)/third_party/actor-framework/libcaf_io/libcaf_io.gyp:*',
      '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
      'follow_trade',
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
