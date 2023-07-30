import matplotlib.pyplot as plt
import numpy as np

# seq_time = 566897566

total = np.loadtxt('small_file.txt')
print(total.shape)

seq = total[0:11]
ff = total[11:22]
thr = total[22:33]


  

plt.scatter(range(11), seq, label='seqential', marker='x')
plt.scatter(range(11), ff, label='FastFlow', marker='v')
plt.scatter(range(11), thr, label='thread', marker='o')

plt.xticks(range(11), ['2^10', '2^11', '2^12', '2^13', '2^14', '2^15', '2^16', '2^17', '2^18', '2^19', '2^20'])

plt.xlabel('number of characters'), 
plt.ylabel('time to complete')

plt.legend()

plt.savefig('small.png')


