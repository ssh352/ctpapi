{
  'includes':[
    '../build/win_precompile.gypi',
  ],
  'targets' : [
    {
      'target_name' : 'ctp_bind',
      'type' : '<(component)',
      'variables' : {
      },
      'sources' : [
       # '<!@(python ../build/glob_files.py . *.h *.cc)',
      ],
      'sources!':[
        'main.cc'
      ],
      'dependencies' : [
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
      'target_name' : 'ctp_bind_demo',
      'type' : 'executable',
      'variables' : {
      },
      'sources' : [
        'main.cc'
      ],
      'dependencies' : [
        'ctp_bind',
        '<(DEPTH)/third_party/ctpapi/ctpapi.gyp:*',
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
