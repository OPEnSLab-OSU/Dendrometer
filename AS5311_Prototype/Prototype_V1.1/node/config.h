"{\
  'general':\
  {\
    'name':'Dendro',\
    'instance':1,\
    'interval':2000\
  },\
	'components':[\
    {\
      'name':'Analog',\
      'params':[8,12,false,false,false,false,false,false,0,0,0,0,0,0,25]\
    },\
    {\
      'name':'SHT31D',\
      'params':'default'\
    },\
    {\
      'name':'DS3231',\
      'params':[10, false, true]\
    },\
    {\
      'name':'Interrupt_Manager',\
      'params':[0]\
    },\
    {\
      'name':'Sleep_Manager',\
      'params':[true,false,1]\
    },\
    {\
      'name':'SD',\
      'params':[true,1000,10,'dend',true]\
    },\
    {\
      'name':'BatchSD',\
      'params':[true,1000,10]\
    },\
    {\
      'name':'Neopixel',\
      'params':'default'\
    },\
    {\
      'name':'LoRa',\
      'params':[\
        255,\
        2,\
        23,\
        15,\
        500\
      ]\
    }\
  ]\
}"
