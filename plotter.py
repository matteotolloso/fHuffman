import matplotlib.pyplot as plt
import numpy as np

# seq_time = 566897566

total = np.loadtxt('speedups.txt')
print(total.shape)

seq = total[0:9]
ff = total[9:19]
thr = total[19:28]


  

plt.scatter(range(9), seq, label='seqential', marker='x')
plt.scatter(range(9), ff, label='FastFlow', marker='v')
plt.scatter(range(9), thr, label='thread', marker='o')

plt.xticks(range(9), ['2^25', 
                       '2^26',
                       '2^27',
                       '2^28',
                       '2^29',
                       '2^30',
                       '2^31',
                       '2^32',
                       '2^33',
                       ])

plt.xlabel('number of characters'), 
plt.ylabel('time to complete')

plt.legend()

plt.savefig('small.png')


