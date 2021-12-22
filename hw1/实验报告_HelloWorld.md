# 操作系统实验一

> 191250128 孙钰昇

## Hello OS

- 在windows上使用VMware WorkStation安装Ubuntu16.04
- 完成环境配置
- 编写代码

```apl
    org 07c00h
	mov ax, cs
	mov ds, ax
	mov es, ax
	call DispStr
	jmp $
DispStr:
	mov ax, BootMessage
	mov bp, ax
	mov cx, 16
	mov ax, 01301h
	mov bx, 000ch
	mov dl, 0
	int 10h
	ret
BootMessage:	db "Hello,OS"
times 510-($-$$)	db 0
dw 0xaa55
```

- 显示结果

![](C:\Users\rubisco\Desktop\大三笔记\操作系统\作业\hw1\pics\QQ截图20211004171710.png)

## 汇编实践

