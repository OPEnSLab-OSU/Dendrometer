"{\
	'general':\
	{\
		'name':'Device',\
		'instance':1\
	},\
	'components':[\
		{\
			'name':'LoRa',\
			'params':[255,1,23,3,200]\
		},\
    {\
     'name':'LTE',\
      'params':['hologram','','','A5']\
    },\
    {\
      'name':'GoogleSheets',\
      'params':[\
        'Goog',\
        7003,\
        '/macros/s/AKfycbz4FDWcWILelj6wwShxXRDyyXOWeiYMZ6KqAjM9f4AW_DukHMMZ/exec',\
        '1hVE12IatRQ3P8pAppXuwFszosDbhLLTj-kEZtR2gL3o',\
/*true to autoname tab*/        true,\
/*not used if previous param is true*/  ''\
      ]\
    },\
		{\
			'name':'SD',\
			'params':[true,1000,10,'dend',true]\
		},\
    {\
      'name':'BatchSD',\
      'params': [true, 1000, 10]\
    }\
	]\
}"
