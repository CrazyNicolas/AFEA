import logging
# 创建一个logging对象
logger = logging.getLogger()

# 创建一个handler，用于写入日志文件
fh = logging.FileHandler('sos.log', mode='w', encoding='utf-8')

# 再创建一个handler用于输出到控制台
#ch = logging.StreamHandler()

# 定义输出格式(可以定义多个输出格式例formatter1，formatter2)
#formatter = logging.Formatter('%(asctime)s %(levelname)s %(filename)s [line %(lineno)d]')

# 定义日志输出层级
logger.setLevel(logging.DEBUG)

# 定义控制台输出层级
#logger.setLevel(logging.DEBUG)

# 为文件操作符绑定格式（可以绑定多种格式例fh.setFormatter(formatter2)）
#fh.setFormatter(formatter)

# 为控制台操作符绑定格式（可以绑定多种格式例ch.setFormatter(formatter2)）
#ch.setFormatter(formatter)

# 给logger对象绑定文件操作符
logger.addHandler(fh)

# 给logger对象绑定文件操作符
#logger.addHandler(ch)

# 错误信息
logger.info(1)
logger.info('{:.2f}'.format(3.1415926))
logger.info('\n')
logger.info('产生错误信息')
logger.info('产生严重错误')

