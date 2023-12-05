# Exports all data from collections in UBC_Dendrometer database
import subprocess
import time

# While loop to iterate data downloading process for each dendrometer collection
i = 1

# There are 72 dendrometers in total
while i < 73:
  
    collection_number = i

    # We will concatenate the dendrometer number to the strings
    collections = "UBC_Dendrometer_" + str(collection_number)
    outputs = "data_dendrometer_" + str(collection_number) + ".csv"
  
    # Specify the fields for the command: replace **USERNAME** and **PASSWORD** with username and password - ask Fredi or Dajeong
    connection_string = 'mongodb+srv://**USERNAME**:**PASSWORD**@remotetest.cls7o.mongodb.net/?retryWrites=true&w=majority'
    database_name = 'UBC_Dendrometer'
    collection_name = collections
    output_file = outputs
    data_fields = '_id,Timestamp.time_local,displacement.um,Analog.Vbat'
    file_type = 'csv'

    # Build the mongoexport command
    command = [
        'mongoexport',
        '--uri', connection_string,
        '--db', database_name,
        '--collection', collection_name,
        '--type', file_type,
        '--fields', data_fields,
        '--out', output_file
    ]

    # Execute the command
    try:
        subprocess.run(command, check=True)
        print('All data exported successfully!')
    except subprocess.CalledProcessError as e:
        print(f'Error: {e.stderr.decode()}')

    # Increment the dendrometer number
    i += 1
  
# Check the current local time
current_time = time.strftime('%Y-%m-%d %H:%M:%S')

# Write the time into a text file
with open('data_download_tracker.txt', 'w') as txtfile:

    txtfile.write(f"Data was last downloaded: {current_time}\n\n")

# You will need an active mongodb service running on your device to run the command
