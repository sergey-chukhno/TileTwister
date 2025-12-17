import wave
import math
import struct
import os

def create_tone(filename, frequency, duration_sec, volume=0.5):
    sample_rate = 44100
    n_frames = int(sample_rate * duration_sec)
    
    with wave.open(filename, 'w') as wav_file:
        wav_file.setnchannels(1) # Mono
        wav_file.setsampwidth(2) # 2 bytes per sample (16-bit)
        wav_file.setframerate(sample_rate)
        
        data = []
        for i in range(n_frames):
            t = float(i) / sample_rate
            # Simple Sine Wave
            value = int(32767.0 * volume * math.sin(2.0 * math.pi * frequency * t))
            data.append(struct.pack('<h', value))
            
        wav_file.writeframes(b''.join(data))
    print(f"Generated {filename}")

# Create assets directory if not exists
if not os.path.exists("assets"):
    os.makedirs("assets")

# Generate Placeholders
create_tone("assets/move.wav", 200, 0.1, 0.3)      # Low blip
create_tone("assets/merge.wav", 400, 0.15, 0.4)    # Mid blip
create_tone("assets/spawn.wav", 600, 0.1, 0.2)     # High pop
create_tone("assets/score.wav", 800, 0.3, 0.2)     # Chime
create_tone("assets/invalid.wav", 150, 0.1, 0.5)   # Low thud (sawtooth ideal but sine ok)
create_tone("assets/gameover.wav", 100, 1.0, 0.5)  # Long low tone

print("All sounds generated.")
