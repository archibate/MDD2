# Perf Tools

```bash
sudo apt install linux-tools-5.15.0-141-generic linux-cloud-tools-5.15.0-141-generic
sudo apt install linux-tools-generic linux-cloud-tools-generic linux-tools-common
sudo apt install valgrind kcachegrind
sudo git clone https://github.com/brendangregg/FlameGraph.git -o /opt/FlameGraph
echo -1 | sudo tee /proc/sys/kernel/perf_event_paranoid
echo 0 | sudo tee /proc/sys/kernel/kptr_restrict
```
