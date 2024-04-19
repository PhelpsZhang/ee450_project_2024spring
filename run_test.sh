#!/bin/bash

# Check the Port number occupied and Kill
PORTS=(41089 42089 43089 44089 45089)

for port in "${PORTS[@]}"; do
    # ss TCP UDP, getPID
    PIDS=$(ss -luntp | grep ":$port " | awk '{print $7}' | sed -e 's/.*pid=//g' -e 's/,.*//g')
    if [ ! -z "$PIDS" ]; then
        echo "Killing processes on port $port: $PIDS"
        for pid in $PIDS; do
            kill $pid
            if [ $? -eq 0 ]; then
                echo "Process $pid on port $port killed."
            else
                echo "Failed to kill process $pid on port $port."
            fi
        done
    else
        echo "No process found on port $port."
    fi
done

# start first program. set window name and keep it 
gnome-terminal -- bash -c 'echo -ne "\033]0;serverM\007"; ./serverM; exec bash'

gnome-terminal -- bash -c 'echo -ne "\033]0;serverS\007"; ./serverS; exec bash'

gnome-terminal -- bash -c 'echo -ne "\033]0;serverD\007"; ./serverD; exec bash'

gnome-terminal -- bash -c 'echo -ne "\033]0;serverU\007"; ./serverU; exec bash'

gnome-terminal -- bash -c 'echo -ne "\033]0;clientA\007"; ./client; exec bash'

gnome-terminal -- bash -c 'echo -ne "\033]0;clientB\007"; ./client; exec bash'