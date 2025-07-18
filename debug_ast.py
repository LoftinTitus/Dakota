#!/usr/bin/env python3
import subprocess
import sys

def run_dakota_and_analyze():
    # Run dakota with the debug control flow test
    result = subprocess.run(['./bin/dakota', 'dakota tests/debug_control_flow.dk'], 
                          capture_output=True, text=True)
    
    print("Return code:", result.returncode)
    print("\nSTDOUT:")
    print(result.stdout)
    print("\nSTDERR:")
    print(result.stderr)

if __name__ == "__main__":
    run_dakota_and_analyze()
