# Путь к заголовочным файлам
message(Settings " CMAKE_SOURCE_DIR")
set(INCLUDE_PATH ${CMAKE_SOURCE_DIR}/include)

# Путь к искодным текстам программ
set(SOURCE_PATH ${CMAKE_SOURCE_DIR}/source)

# Путь к собранным исполняемым модулям
set(BIN_PATH ${CMAKE_SOURCE_DIR}/bin)

# Путь к собранным статическим библиотекам
set(LIB_PATH ${CMAKE_SOURCE_DIR}/libs)

# Путь к сторонним библиотекам
set(3RDPARTY_PATH ${CMAKE_SOURCE_DIR}/3rdParty)

# Путь к конфигурационным файлам проекта
set(CONFIG_PATH ${CMAKE_SOURCE_DIR}/config)

# Путь к числовым данным
set(DATA_PATH ${CMAKE_SOURCE_DIR}/data)

# Путь к библиотеке boost
#set(BOOST_PATH D:/Work/Libs/boost/1.76/Build.dir)
set(BOOST_PATH D:/boost_1_82_0)

# Путь к библиотеке gpdraw
set(GPDRAW_PATH D:/Work/Libs/gpdraw/1.1.3)

# Путь к библиотеке iniparser
set(INIPARSER_PATH D:/Work/Libs/iniparser)

# Путь к библиотеке LibJsonParam
set(JSON_PARAM_PATH D:/Work/Libs/LibJsonParam)

# Путь к библиотеке json
set(JSON_PATH D:/Work/Libs/json)

# Путь к библиотеке science
set(SCIENCE_PATH D:/Work/Libs/science/2.5.4)

# Путь к библиотеке WorkerZMQ
set(WORKER_ZMQ_PATH D:/Work/Libs/WorkerZMQ)

# Путь к библиотеке zmq
set(ZMQ_PATH D:/Work/Libs/zmq)