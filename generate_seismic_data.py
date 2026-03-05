import csv
import math
import random

# --- Global Settings ---
SAMPLE_RATE = 100  # 100 Hz sampling rate
DURATION = 10      # 10 seconds of data per file
TOTAL_SAMPLES = SAMPLE_RATE * DURATION

def generate_elephant_data():
    """Generates a 20Hz, high-amplitude sine wave with minor noise."""
    with open('Elephant.csv', mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['timestamp', 'z_axis'])
        
        for i in range(TOTAL_SAMPLES):
            t_ms = i * (1000 / SAMPLE_RATE)
            t_sec = t_ms / 1000.0
            # 3.0g amplitude, 20Hz frequency + small random noise
            z_axis = 3.0 * math.sin(2 * math.pi * 20.0 * t_sec) + random.uniform(-0.1, 0.1)
            writer.writerow([int(t_ms), round(z_axis, 4)])
    print("✅ Elephant.csv generated successfully! (20Hz High-Amplitude)")

def generate_human_data():
    """Generates a 5Hz, medium-amplitude sine wave with irregular noise."""
    with open('Human.csv', mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['timestamp', 'z_axis'])
        
        for i in range(TOTAL_SAMPLES):
            t_ms = i * (1000 / SAMPLE_RATE)
            t_sec = t_ms / 1000.0
            # 1.5g amplitude, 5Hz frequency + medium random noise
            z_axis = 1.5 * math.sin(2 * math.pi * 5.0 * t_sec) + random.uniform(-0.4, 0.4)
            writer.writerow([int(t_ms), round(z_axis, 4)])
    print("✅ Human.csv generated successfully! (5Hz Medium-Amplitude)")

def generate_background_data()
    """Generates pure random environmental noise."""
    with open('Background.csv', mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['timestamp', 'z_axis'])
        
        for i in range(TOTAL_SAMPLES):
            t_ms = i * (1000 / SAMPLE_RATE)
            # Pure random noise between -0.1g and 0.1g
            z_axis = random.uniform(-0.1, 0.1)
            writer.writerow([int(t_ms), round(z_axis, 4)])
    print("✅ Background.csv generated successfully! (Low-Amplitude Noise)")

# --- Run the Generators ---
if __name__ == "__main__":
    print("🛠️ Generating Synthetic Seismic Data for Edge Impulse...")
    generate_elephant_data()
    generate_human_data()
    generate_background_data()
    print("🎉 All files are ready to be uploaded to Edge Impulse Studio!")