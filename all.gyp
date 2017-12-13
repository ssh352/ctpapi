{
  'targets' : [
    {
      'target_name' : 'all',
      'type' : 'none',
      'variables' : {
      },
      'sources' : [
        'common/api_struct.h',
        'common/api_data_type.h',
      ],
      'dependencies' : [
        './ctp_broker/ctp_broker.gyp:*',
        './hpt_core/hpt_core.gyp:*',
        './live_trade/live_trade.gyp:*',
        './strategy_unittest/strategy_unittest.gyp:*',
      ],
    },
  ]
}
