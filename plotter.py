import matplotlib.pyplot as plt
import numpy as np

# seq_time = 566897566

thr = np.loadtxt('efficiency_thr')
ff = np.loadtxt('efficiency_ff')

thr = thr / 1e6
ff = ff / 1e6


# thr = seq_time/thr
# ff = seq_time/ff

# cores = [i for i in range(4,33,2)]

# thr = thr / cores
# ff = ff / cores
  

plt.scatter(range(4,33,2),thr, label='thread', marker='o')
plt.scatter(range(4,33,2),ff, label='FastFlow', marker='v')

plt.xticks(range(4,33,2))

plt.xlabel('number of cores'), 
plt.ylabel('time to complete')

plt.legend()

plt.savefig('efficiency.png')


