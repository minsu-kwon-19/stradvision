import socket
import struct
import time

def inject_fault():
    print("Fault Injector: Connecting to controller on port 9000...")
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('localhost', 9000))
        print("Connected. Currently this is a stub.")
        # Future: generate a fake STATE packet with load = 99.9
        # to trigger the policy engine.
        time.sleep(1)
        s.close()
    except Exception as e:
        print(f"Error: {e}")

if __name__ == "__main__":
    inject_fault()
