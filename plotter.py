import matplotlib.pyplot as plt
import numpy as np

seq_time = 566897566

thr = np.loadtxt('efficiency_ff')


for i in range(2, len(thr), 2):
    thr[i] = (seq_time * i) / thr[i]
  

plt.plot(thr)

plt.savefig('efficiency.png')


