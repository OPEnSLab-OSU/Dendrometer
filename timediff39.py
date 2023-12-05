#########################################################
#   Manual time shift by inspection for dendrometer_39  #
#########################################################

import csv
from datetime import datetime, timedelta

# Function to calculate time difference between two timestamps
def calculate_time_difference(timestamp1, timestamp2):

    # Specify datetime format
    time_format = "%Y-%m-%d %I:%M:%S %p"

    # Parse input strings to specified format
    time1 = datetime.strptime(timestamp1, time_format)
    time2 = datetime.strptime(timestamp2, time_format)

    # Calculate time difference and return result
    time_diff = time2 - time1
    return time_diff

# Specify file to read from and write to
file_path = "C:/Users/kimda/Onedrive/Documents/PhaenoFlex/data/2023/magnetic_SD/39/data_combined_39.csv"
output_path = "C:/Users/kimda/Onedrive/Documents/PhaenoFlex/data/2023/magnetic_SD/39/shifted_data_39.csv"


# The row numbers from which you want to start modifying timestamps
start_row_1 = 135
start_row_2 = 516
start_row_3 = 2066
start_row_4 = 3318
start_row_5 = 3415

# Read the CSV file and update timestamps
with open(file_path, 'r') as csv_file:
    csv_reader = csv.reader(csv_file)
    rows = list(csv_reader)

# Get timestamps
timestamp_row_135 = rows[135][3]  # Assuming timestamps are in column 4 (index 3)
timestamp_row_136 = rows[134][3]

# Calculate time difference for first time shift
time_difference = calculate_time_difference(timestamp_row_135, timestamp_row_136) + timedelta(minutes=15)

# Get timestamps
timestamp_row_516 = rows[516][3] 
timestamp_row_517 = rows[515][3]

# Calculate time difference for first time shift
time_difference2 = calculate_time_difference(timestamp_row_516, timestamp_row_517) + timedelta(minutes=15)

# Get timestamps
timestamp_row_2066 = rows[2066][3]  
timestamp_row_2067 = rows[2065][3]

# Calculate time difference for first time shift
time_difference3 = calculate_time_difference(timestamp_row_2066, timestamp_row_2067) + timedelta(minutes=15)

# Get timestamps
timestamp_row_3318 = rows[3318][3]
timestamp_row_3319 = rows[3317][3]

# Calculate time difference for first time shift
time_difference4 = calculate_time_difference(timestamp_row_3318, timestamp_row_3319) + timedelta(minutes=15)

# Get timestamps
timestamp_row_3415 = rows[3415][3] 
timestamp_row_3416 = rows[3414][3]

# Calculate time difference for first time shift
time_difference5 = calculate_time_difference(timestamp_row_3415, timestamp_row_3416) + timedelta(minutes=15)

# Update timestamps starting from start_row_1
for i in range(start_row_1, start_row_2):
    current_timestamp = rows[i][3]
    updated_timestamp = (datetime.strptime(current_timestamp, "%Y-%m-%d %I:%M:%S %p") + time_difference).strftime("%Y-%m-%d %I:%M:%S %p")
    rows[i][3] = updated_timestamp

# Update timestamps starting from start_row_2
for i in range(start_row_2, start_row_3):
    current_timestamp2 = rows[i][3]
    updated_timestamp2 = (datetime.strptime(current_timestamp2, "%Y-%m-%d %I:%M:%S %p") + time_difference + time_difference2).strftime("%Y-%m-%d %I:%M:%S %p")
    rows[i][3] = updated_timestamp2

# Update timestamps starting from start_row_3
for i in range(start_row_3, start_row_4):
    current_timestamp3 = rows[i][3]
    updated_timestamp3 = (datetime.strptime(current_timestamp3, "%Y-%m-%d %I:%M:%S %p") + time_difference + time_difference2 + time_difference3).strftime("%Y-%m-%d %I:%M:%S %p")
    rows[i][3] = updated_timestamp3

# Update timestamps starting from start_row_4
for i in range(start_row_4, start_row_5):
    current_timestamp4 = rows[i][3]
    updated_timestamp4 = (datetime.strptime(current_timestamp4, "%Y-%m-%d %I:%M:%S %p") + time_difference + time_difference2 + time_difference3 + time_difference4).strftime("%Y-%m-%d %I:%M:%S %p")
    rows[i][3] = updated_timestamp4

# Update timestamps starting from start_row_5
for i in range(start_row_5, len(rows)):
    current_timestamp5 = rows[i][3]
    updated_timestamp5 = (datetime.strptime(current_timestamp5, "%Y-%m-%d %I:%M:%S %p") + time_difference + time_difference2 + time_difference3 + time_difference4 + time_difference5).strftime("%Y-%m-%d %I:%M:%S %p")
    rows[i][3] = updated_timestamp5

# Write the modified data to a new CSV file
with open(output_path, 'w', newline='') as csv_output:
    csv_writer = csv.writer(csv_output)
    csv_writer.writerows(rows)

# Done!
print("Timestamps updated and saved to:", output_path)
