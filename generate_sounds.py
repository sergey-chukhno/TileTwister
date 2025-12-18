
import wave
import math
import struct
import random
import os

def save_wav(filename, samples, sample_rate=44100):
    with wave.open(filename, 'w') as obj:
        obj.setnchannels(1) # mono
        obj.setsampwidth(2) # 2 bytes
        obj.setframerate(sample_rate)
        
        data = bytearray()
        for s in samples:
            s = max(-1.0, min(1.0, s))
            val = int(s * 32767.0)
            data += struct.pack('<h', val)
            
        obj.writeframes(data)
    print(f"Generated {filename}")

def generate_fireworks(duration_sec=4.0, sample_rate=44100):
    samples = [0.0] * int(duration_sec * sample_rate)
    
    # Generate multiple bursts
    num_bursts = 5
    for b in range(num_bursts):
        start_time = random.uniform(0.0, 2.5) # Start within first 2.5s
        
        # Each burst
        burst_dur = 1.5
        burst_samples = int(burst_dur * sample_rate)
        
        pitch_shift = random.uniform(0.8, 1.2)
        
        for i in range(burst_samples):
            t = i / sample_rate
            idx = int(start_time * sample_rate) + i
            if idx >= len(samples): break
            
            # Noise + Crackle
            noise = random.uniform(-1.0, 1.0)
            
            # Envelope: Sharp attack, long decay
            if t < 0.05:
                env = t / 0.05
            else:
                env = math.exp(-(t - 0.05) * 4.0)
            
            # Crackle
            crackle = 0.0
            if t > 0.1 and random.random() < 0.02 * env:
                crackle = random.uniform(-0.9, 0.9)
                
            val = (noise * 0.7 + crackle) * env * 0.6 # reduce amp to avoid clip
            samples[idx] += val

    return samples

if __name__ == "__main__":
    if not os.path.exists("assets"):
        os.makedirs("assets")
        
    fw = generate_fireworks()
    save_wav("assets/fireworks.wav", fw)
