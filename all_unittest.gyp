{
  'targets' : [
    {
      'target_name' : 'all_unittest',
      'type' : 'none',
      'variables' : {
      },
      'sources' : [
      ],
      'dependencies' : [
        './ctp_broker/ctp_broker.gyp:ctp_broker_unittest',
        './hpt_core/hpt_core.gyp:hpt_core_unittest',
        # 'live_trade/live_trade.gyp:*',
        # 'strategies/strategies.gyp:*',
        './strategy_unittest/strategy_unittest.gyp:*',
      ],
    },
  ]
}
