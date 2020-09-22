from collections import namedtuple
from utils.file_utils import save_pkl, load_pkl
from utils.log_utils import debug, info
from typing import Callable, Any,List
from utils.container_utils import shuffle_list

'''
video 66*65
iot 66*65
voip 66*65
ar 66*65
labels=dict{
"video":66*65,list,
"iot":66*65,list,
"voip":66*65,list,
"ar":66*65,list
}
'''
ILPInstance = namedtuple("ILPInstance", ["video", "iot", "voip", "ar", "labels"])


def map_instance(instances: [ILPInstance], map_func: Callable[[ILPInstance], Any], ratio=0.7,
                 shuffle=False)->(List[Any],List[Any]):
	'''

	:param instances: list of ilpinstance
	:param map_func: function to map single ilp instance to any obj
	:param ratio: ratio=n_train/n_all
	:param shuffle: shuffle before mapping
	:return:
	'''
	if shuffle:
		instances = shuffle_list(instances)

	n_instances = len(instances)
	info("got {} instances".format(n_instances))
	n_train = int(n_instances) * ratio
	n_test = n_instances - n_train
	info("#train:{},#test:{}", n_train, n_test)

	train=[]
	test=[]
	for i in instances[:n_train]:
		train.append(map_func(i))

	for i in instances[n_train:]:
		test.append(map_func(i))
	return train,test

'''
ilpinput
video list 66*65
iot list 66
'''
ILPInput=namedtuple("ILPInput",["video","iot","voip","ar"])
ILPOutput=namedtuple("ILPOutput",["video","iot","voip","ar"])

if __name__ == '__main__':
	instance = ILPInstance([], [], [], [], {})
	save_pkl("/tmp/demo.pkl", instance)
	instance = load_pkl("/tmp/demo.pkl")