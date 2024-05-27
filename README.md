### Hi there 👋

**moboware/moboware** is a ✨ _special_ ✨ repository because its `README.md` (this file) appears on your GitHub profile.

**Checking performance with gprof**
- Set the cmake option: PERFORMANCE_BUILD_FLAGS:On
- run the application for a moment and stop nicely
- check the gmon.out file with : gprof --inline-file-names --all-lines  --static-call-graph <your application> gmon.out | less

**Checking perfomance with perf**
- Build in release mode with the option: PERFORMANCE_BUILD_FLAGS:On
- Use hotspot to run you application and check the perf output file