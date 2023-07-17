#!/bin/bash
calculate_time_diff() {
    file=$1
    field_number=$2

    # Extract first and last timestamps
    first_timestamp=$(head -n 1 "$file" | awk -F' - ' -v num=$field_number '{print $num}')
    last_timestamp=$(tail -n 1 "$file" | awk -F' - ' -v num=$field_number '{print $num}')

    # Convert timestamps to Unix timestamps (in seconds)
    first_unix_timestamp=$(date -d "$first_timestamp" +%s)
    last_unix_timestamp=$(date -d "$last_timestamp" +%s)

    # Calculate difference in milliseconds
    diff=$(echo "($last_unix_timestamp - $first_unix_timestamp) * 1000" | bc)

    echo "$diff"
}

OUTPUTS_FOLDER="outputs"
CSV_DATA_FOLDER="csv_data"
CSV_FILE="csv_data.csv"
# # Remove the existing csv data and logs_and_results folder
# rm -r "$OUTPUTS_FOLDER"
# rm -r "$CSV_DATA_FOLDER"

# # Create folder if it doesn't exist
# mkdir -p "$OUTPUTS_FOLDER"
# mkdir -p "$CSV_DATA_FOLDER"

# echo "Running client - r=10, k=2"
# r=10
# k=2
# rm -f csv_data.csv
# echo "n,r,k,valid_log, valid_results, log_time_diff,result_time_diff" >> "$CSV_FILE"  # Add the header line to the CSV file
# for n in 2 4 8 16 32; do
#     rm -f log.txt resultado.txt
#     echo "  n=$n"
#     ./cliente $n $r $k
#     LOG_TIME_DIFF=$(calculate_time_diff "log.txt" 3)
#     RESULT_TIME_DIFF=$(calculate_time_diff "resultado.txt" 2)

#     LOG_VALIDATION=$(python3 log_validator.py)
#     RESULTS_VALIDATION=$(python3 result_validator.py $n $r)

#     VALID_LOG=$( [ "$LOG_VALIDATION" == "Log file was successfully validated" ] && echo "true" || echo "false" )
#     VALID_RESULTS=$( [ "$RESULTS_VALIDATION" == "Result validated" ] && echo "true" || echo "false" )
#     echo "$n, $r, $k, $VALID_LOG, $VALID_RESULTS, $LOG_TIME_DIFF, $RESULT_TIME_DIFF" >> "$CSV_FILE"
#     cp log.txt "$OUTPUTS_FOLDER/log_n${n}_r${r}_k${k}.txt"
#     cp resultado.txt "$OUTPUTS_FOLDER/resultado_n${n}_r${r}_k${k}.txt"
# done
# cp csv_data.csv "$CSV_DATA_FOLDER/csv_data_r${r}_k${k}.csv"

# echo "Running client - r=5, k=1"
# r=5
# k=1
# rm -f csv_data.csv
# echo "n,r,k,valid_log, valid_results, log_time_diff,result_time_diff" >> "$CSV_FILE"  # Add the header line to the CSV file
# for n in 2 4 8 16 32 64; do
#     rm -f log.txt resultado.txt
#     echo "  n=$n"
#     ./cliente $n $r $k
#     LOG_TIME_DIFF=$(calculate_time_diff "log.txt" 3)
#     RESULT_TIME_DIFF=$(calculate_time_diff "resultado.txt" 2)

#     LOG_VALIDATION=$(python3 log_validator.py)
#     RESULTS_VALIDATION=$(python3 result_validator.py $n $r)

#     VALID_LOG=$( [ "$LOG_VALIDATION" == "Log file was successfully validated" ] && echo "true" || echo "false" )
#     VALID_RESULTS=$( [ "$RESULTS_VALIDATION" == "Result validated" ] && echo "true" || echo "false" )
#     echo "$n, $r, $k, $VALID_LOG, $VALID_RESULTS, $LOG_TIME_DIFF, $RESULT_TIME_DIFF" >> "$CSV_FILE"
#     cp log.txt "$OUTPUTS_FOLDER/log_n${n}_r${r}_k${k}.txt"
#     cp resultado.txt "$OUTPUTS_FOLDER/resultado_n${n}_r${r}_k${k}.txt"
# done
# cp csv_data.csv "$CSV_DATA_FOLDER/csv_data_r${r}_k${k}.csv"

# echo "Running client - r=3, k=0"
# r=3
# k=0
# rm -f csv_data.csv
# echo "n,r,k,valid_log, valid_results, log_time_diff,result_time_diff" >> "$CSV_FILE"  # Add the header line to the CSV file
# for n in 2 4 8 16 32 64 128; do
#     rm -f log.txt resultado.txt
#     echo "  n=$n"
#     ./cliente $n $r $k
#     LOG_TIME_DIFF=$(calculate_time_diff "log.txt" 3)
#     RESULT_TIME_DIFF=$(calculate_time_diff "resultado.txt" 2)

#     LOG_VALIDATION=$(python3 log_validator.py)
#     RESULTS_VALIDATION=$(python3 result_validator.py $n $r)

#     VALID_LOG=$( [ "$LOG_VALIDATION" == "Log file was successfully validated" ] && echo "true" || echo "false" )
#     VALID_RESULTS=$( [ "$RESULTS_VALIDATION" == "Result validated" ] && echo "true" || echo "false" )
#     echo "$n, $r, $k, $VALID_LOG, $VALID_RESULTS, $LOG_TIME_DIFF, $RESULT_TIME_DIFF" >> "$CSV_FILE"
#     cp log.txt "$OUTPUTS_FOLDER/log_n${n}_r${r}_k${k}.txt"
#     cp resultado.txt "$OUTPUTS_FOLDER/resultado_n${n}_r${r}_k${k}.txt"
# done
# cp csv_data.csv "$CSV_DATA_FOLDER/csv_data_r${r}_k${k}.csv"

gnuplot plot_graphics.gp