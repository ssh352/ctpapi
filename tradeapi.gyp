{
  'targets' : [
    {
      'target_name' : 'tradeapi',
      'type' : 'none',
      'variables' : {
      },
      'sources' : [
        '<!@(python <(DEPTH)/build/glob_files.py inlcude/tradeapi *.h)',
      ],
      'dependencies' : [
      ],
      'defines' : [
      ],
      'includes' : [
      ],
      'include_dirs' : [
      ],
    },
  ]
}
