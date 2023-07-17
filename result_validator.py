import collections
import argparse

def validate(n, r):
    prev_timestamp = None
    line_count = 0
    process_ids = []

    with open("resultado.txt", "r") as file:
        for line in file:
            process_id = int(line.split(" - ")[0].split(" ")[1])
            timestamp = line.split(" - ")[1]
            line_count += 1
            process_ids.append(process_id)

            if prev_timestamp is not None and prev_timestamp > timestamp:
                s = "Invalid result: timestamps are not in chronological order!"
                print(s)
                return s

            prev_timestamp = timestamp

    expected_line_count = n * r
    if line_count != expected_line_count:
        s = f"Invalid result: expected {expected_line_count} lines, but found {line_count} lines."
        print(s)
        return s

    process_counts = dict(collections.Counter(process_ids))
    for process_id, count in process_counts.items():
        if count != r:
            s = f"Invalid result: Process ID {process_id} is not present {r} times."
            print(s)
            return s

    s = "Result validated"
    print(s)
    return s

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("n", type=int, help="Value for n")
    parser.add_argument("r", type=int, help="Value for r")
    args = parser.parse_args()

    validate(args.n, args.r)