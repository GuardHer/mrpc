`
vscode: 
调试快捷键: Ctrl + F5
更换调试目标: Ctrl + Alt + F5
`



git 21.08

### 日志模块
日志格式
```
[level][%y-%m-%d %H:%M:%S.%ms]\t[pid:thread_id]\t[file_name:line][%msg]
```

### Logger 日志器
1.提供打印日志的方法
2.

```

```

### net 网络

#### EventLoop 事件循环


#### TimerEvent 定时任务
1、arrive_time   ：指定时间点 
2、interval, ms  ：间隔
3、is_repeated   ：是否重复
4、is_cancled	 ：取消
5、task

cancle()         ：取消定时任务
cancleRepeated() ：取消定时任务的重复属性


#### Timer
`定时器，他是一个TimerEvent的集合`
`Timer 继承自 FdEvent`

```
addTimerEvent()
delTimerEvent()
onTimer()

reserArriveTime()

multimap 存储 TimerEvent <key(arrive_time), TimerEvent>
```

#### IO线程
1、创建一个新线程
2、在新线程里面创建一个EventLoop，完成初始化
3、开启loop

```
 class {
	
	pthread_t m_thread;
	int32_t m_tid;
	EventLoop event_loop;
 }
```