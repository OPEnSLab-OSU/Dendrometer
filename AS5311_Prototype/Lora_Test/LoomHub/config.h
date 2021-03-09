"{\
	'general':\
	{\
		'name':'Device',\
		'instance':1,\
		'interval':10000\
	},\
	'components':[\
		{\
			'name':'Analog',\
			'params':'default'\
		},\
		{\
			'name':'Digital',\
			'params':'default'\
		},\
		{\
			'name':'Ethernet',\
			'params':[\
				'Ether1',\
				[134,171,186,10,33,221],\
				[192,168,0,1]\
			]\
		},\
		{\
			'name':'GoogleSheets',\
			'params':[\
				'Goog',\
				7001,\
				'/macros/s/AKfycbzySpouxdaHYh6f1e7DI24i4s8XTGDe-X6d-9uaR7HVrpFNWUt7/exec',\
				'11bmZETLyFutZHwZRrpmsNFy0VrWmJ-myskjLZ6cZS-w',\
/*true to autoname tab*/				true,\
/*not used if previous param is true*/	'testTab2'\
			]\
		}\
	]\
}"