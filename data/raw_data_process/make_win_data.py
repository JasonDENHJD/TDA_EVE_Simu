# This is a sample Python script.

# Press Shift+F10 to execute it or replace it with your code.
# Press Double Shift to search everywhere for classes, files, tool windows, actions, and settings.

import numpy as np
import scipy.signal as signal
def print_hi(name):
    # Use a breakpoint in the code line below to debug your script.
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.


# Press the green button in the gutter to run the script.
lens = [64, 128, 256, 512, 1024]
precs = ['uint8', 'int8', 'uint16','int16', 'uint32', 'int32']
if __name__ == '__main__':
    # data_name = '../20220121_090433_mulit.npz'
    # data = np.load(data_name)['lower_4g']
    # # data.tofile(data_name[:-4] + '.bin')
    # print_hi('PyCharm')
    for length in lens:
        for prec in precs:
            win_data = signal.windows.hann(length) * np.iinfo(prec).max
            win_data = win_data.astype(prec)
            win_data.tofile('../hann/hann_' + str(length) + '_' + prec + '.bin')

# See PyCharm help at https://www.jetbrains.com/help/pycharm/
