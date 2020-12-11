"{\
	'general':\
	{\
		'name':'Device',\
		'instance':1,\
		'interval':1000\
	},\
	'components':[\
		{\
			'name':'SHT31D',\
			'params':'default'\
		},\
		{\
			'name':'SD',\
			'params':[true,1000,10,'dend',true]\
		},\
		{\
			'name':'DS3231',\
			'params':[11, true, true]\
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
        	'name':'Neopixel',\
        	'params':'default'\
        }\
	]\
}"