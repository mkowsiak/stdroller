# stdroller

Simple tool to roll over the logs. You can use it to redirect your stdout/stderr and create rolling log file.

### to build
    make

### to run
    some_cmd | ./stdroller -f some_file_name
    
### currently supported options

    usage:
      stdroller --file=file_name|-f file_name [--sufix|-s] [--limit=size|-l size]
                                              [--help|-h] [--rollover|-r]
    
    options:
    
      -f file_name
      --file=file_name    - File name where log will be stored
    
      -s
      --sufix             - Should I create suffix files or not?
                            If you want me to create suffix files
                            I will create each new suffix file after limit
                            is reached. Otherwise, I will overwrite
                            oryginal log. I mean, I will overwrite it
                            like destroying it, cleaning, you will get
                            no nothing.
    
      -l limit
      --limit=limit       - Log size limit; default is 1GB. You can use SI prefixes
                            to specify the size of file, e.g.: 10K, 21k, 10G, etc.
    
      -r
      --rollover          - If you set roll over flag, I will not destroy content
                            However, tail -F will not work in this case. So, you
                            are the one to decide which log style you prefer
    
      -h
      --help              - surprize, surprize! I will show you help message.    
