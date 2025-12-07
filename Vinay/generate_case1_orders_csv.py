import csv
import random

# Output CSV file name (put this later into your Vinay/ folder)
OUTPUT_FILE = "cloud-kitchen-orders(case-1).csv"

NUM_ROWS = 10000

header = [
    "order_id",
    "customer_id",
    "area_code",
    "order_time_min",
    "promised_time_min",
    "prep_time_min",
    "penalty_score",
    "order_value",
    "rider_id_assigned",
    "status"
]

area_codes = [f"NODE_{i:02d}" for i in range(1, 31)]  # 30 areas
rider_ids = [f"RID{i:04d}" for i in range(1, 31)]    # 30 riders
statuses = ["PENDING", "COOKING", "OUT_FOR_DELIVERY", "DELIVERED", "CANCELLED"]

with open(OUTPUT_FILE, "w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(header)

    for i in range(1, NUM_ROWS + 1):
        order_id = f"ORD{i:05d}"
        customer_id = f"CUST{random.randint(1, 2000):04d}"

        order_time = random.randint(0, 23 * 60)  # any minute in first 23 hours
        prep_time = random.randint(12, 35)
        promised_time = order_time + random.randint(25, 50)

        penalty_score = round(random.uniform(0.5, 2.0), 2)
        order_value = random.randint(150, 900)

        # 60% assigned to riders, rest -1
        if random.random() < 0.6:
            rider_id = random.choice(rider_ids)
        else:
            rider_id = "-1"

        status = random.choice(statuses)

        row = [
            order_id,
            customer_id,
            random.choice(area_codes),
            order_time,
            promised_time,
            prep_time,
            penalty_score,
            order_value,
            rider_id,
            status
        ]
        writer.writerow(row)

print(f"Generated {NUM_ROWS} rows into {OUTPUT_FILE}")
