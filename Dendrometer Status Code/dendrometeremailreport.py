###################################################
#   Create report with dendrometer information    #
#  < Check  if data is being updated regularly >  #
###################################################

# You will need an active mongodb service running on your device to run the command :)
# Also you need to install pymongo: pip install pymongo
# Run this script with the following command in your command-line: python dendrometeremailreport.py 
# Replace fields **redacted** below with appropriate information

# Import modules
import csv
import smtplib
from pymongo import MongoClient
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

# Specify MongoDB conenction details - ask Fredi or Dajeong for username and password
uri = "mongodb+srv://**USERNAME**:**PASSWORD**@remotetest.cls7o.mongodb.net/UBC_Dendrometer"
database = 'UBC_Dendrometer'

# Create a new client and connect to the server
client = MongoClient(uri)
db = client[database]

# Send a ping to confirm successful connection
try:
    client.admin.command('ping')
    print("Successfully connected to MongoDB!")
except Exception as e:
    print(e)

# Specify columns to read from csv files
id_column = '_id'
Packet_column = 'Packet.Number'
Timestamp_column = 'Timestamp.time_local'

# We will add report content to these variables
report = []
new_line = ''

# Print last data download time from text file record
with open('data_download_tracker.txt', 'r') as file:       

        # Iterate through every line in file
        for line in file:

            # Append content of file to report
            report.append(line)

# Create a while loop to iterate for each collection
i = 1
while i < 73:
  
    collection_number = i

    # Update the collection name
    collection_name = "UBC_Dendrometer_" + str(collection_number)

    # Specify local file to read from
    filename = "data_dendrometer_" + str(collection_number) + ".csv"
  
    # Print Dendrometer Number
    new_line = f"UBC_Dendrometer_{collection_number}\n"

    # Clear variable
    last_entry = None

    # Read last entry from CSV file
    with open(filename, 'r') as file:
        reader = csv.DictReader(file)

        # Iterate through every row in file until last row
        for row in reader:

            # Store the last data information in each variable
            last_entry_id = row[id_column]
            last_entry_packet = row[Packet_column]
            last_entry_timestamp = row[Timestamp_column]
        
    # Retrieve last document from current collection in MongoDB
    collection = db[collection_name]
    last_document = collection.find_one(sort=[('_id', -1)])

    # Compare the values from the downloaded csv files to the data on the database (_id, Packet.Number, Timestamp.time_local)
    if last_document is not None:
        if last_entry_id == str(last_document['_id']) and last_entry_packet == str(last_document['Packet']['Number']) and last_entry_timestamp == str(last_document['Timestamp']['time_local']):
            new_line += f"## Warning: Data has not been updated since last download. ##\n"
        else:
            #new_line += f"Data has been updated since last download.\n"
            pass
    else:
        new_line += "No data found in MongoDB for time comparison.\n"
        
    # Check Alignment
    if last_document['AS5311']['Alignment'] == "Green":
        #new_line += f"Alignment is Green.\n"
        pass 
    else: 
        alignment = last_document['AS5311']['Alignment']
        new_line += f"## Warning: Alignment is not Green. ##\nAlignment: {alignment}\n"

    # Check WiFi.SSID
    if last_document['WiFi']['SSID'] == "ubcvisitor":
        #new_line += f"Wifi is 'ubcvisitor'.\n"
        pass 
    else: 
        wifi = last_document['WiFi']['SSID']
        new_line += f"## Warning: Wifi connection is not 'ubcvisitor'. ##\nSSID: {wifi}\n"

    # Check WiFi.RSSI
    if last_document['WiFi']['RSSI'] == -100:
        new_line += f"## Warning: Wifi is not working. ##\n" 
    else: 
        #new_line += f"Wifi is working.\n"
        pass

    # Check battery voltage lower boundary
    if last_document['Analog']['Vbat'] > 3.6 :
        #new_line += f"Battery V low ok.\n"
        pass 
    else: 
        voltage = last_document['Analog']['Vbat']
        new_line += f"## Warning: Battery voltage LOW. ##\nVoltage: {voltage}V\n"

    # Check battery voltage higher boundary
    if last_document['Analog']['Vbat'] < 4.3 :
        #new_line += f"Battery V high ok.\n"
        pass 
    else: 
        voltage = last_document['Analog']['Vbat']
        new_line += f"## Warning: Battery voltage HIGH. ##\nVoltage: {voltage}V\n"

    # Append new lines to report
    report.append(new_line) 

    # Increment collection number
    i += 1

# End connection to MongoDB
client.close()


# Create email content
email_content = "New Dendro Report for better time warnings:\n\n" + "\n\n".join(report)

# Configure email parameters
sender_email = '**YOUR_EMAIL_ADDRESS**'
receiver_emails = ['**RECEIPIENT_EMAILS**', '**RECEIPIENT_EMAILS**', '**RECEIPIENT_EMAILS**']
subject = 'Dendrometer Voltage Report'

# Set up smtp
smtp_server = 'smtp-mail.outlook.com'
smtp_port = 587
smtp_username = '**YOUR_EMAIL_ADDRESS**'
smtp_password = '**PASSWORD**'

# Send the email
try:
    with smtplib.SMTP(smtp_server, smtp_port) as server:
        server.starttls()
        server.login(smtp_username, smtp_password)
        for receiver_email in receiver_emails:
            # Create the MIMEText email message
            email_message = MIMEMultipart()
            email_message['Subject'] = subject
            email_message['From'] = sender_email
            email_message['To'] = receiver_email
            email_message.attach(MIMEText(email_content, 'plain'))

            server.send_message(email_message)
            print(f"Successfully sent email to {receiver_email}!")
        print("Successfully sent email to all receipients!")

except smtplib.SMTPException as e:
    print(f"An error occured while sending the email: {str(e)}")
