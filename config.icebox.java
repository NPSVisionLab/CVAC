#
# The IceBox ServiceManager object can be provided by configuring
# IceBox.ServiceManager.Endpoints (and optionally
# IceBox.InstanceName), or as a facet of the Ice.Admin object
# (or both if you like). Make sure to keep config.admin
# synchronized with these settings.
#

#
# IceBox.ServiceManager configuration
#

#Ice.Default.EncodingVersion=1.0

#
# The IceBox instance name is used to set the category field of the
# IceBox ServiceManager identity.
#
#IceBox.InstanceName=DemoIceBox

#
# The IceBox server endpoint configuration
#
#IceBox.ServiceManager.Endpoints=tcp -p 9998 -h 127.0.0.1

#
# Ice.Admin configuration
#

#
# Enable Ice.Admin object:
#
Ice.Admin.InstanceName=DemoIceBox
Ice.Admin.Endpoints=tcp -p 9997 -h 127.0.0.1

#
# The hello service
#
#IceBox.Service.MultiBoost=MultiBoostIceService:create --Ice.Config=config.service
#IceBox.Service.CVAC_OpenCV_Detector=OpenCVPerformance:create --Ice.Config=config.service
#IceBox.Service.CVAC_OpenCV_Trainer=OpenCVPerformance:createTrainer --Ice.Config=config.service
#IceBox.Service.SBD_Trainer=SBDICEServerTrain:create --Ice.Config=config.service
#IceBox.Service.SBDTest=SBDICEServerTest:create --Ice.Config=config.service
#IceBox.Service.VideoValidatorForOpenCV=VideoValidatorForOpenCVServer:create --Ice.Config=config.service

# CorpusServer
IceBox.Service.CorpusService=cvac.corpus.CorpusServiceI --Ice.Config=config.service

#
# Warn about connection exceptions
#
#Ice.Warn.Connections=1

#
# Network Tracing
#
# 0 = no network tracing
# 1 = trace connection establishment and closure
# 2 = like 1, but more detailed
# 3 = like 2, but also trace data transfer
#
#Ice.Trace.Network=2
#Ice.Trace.Slicing=3

#
# Protocol Tracing
#
# 0 = no protocol tracing
# 1 = trace protocol messages
#
#Ice.Trace.Protocol=1
