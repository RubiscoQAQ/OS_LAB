section .data
msg: db "Please input X and Y:"
line: db 0Ah
section .bss
input: resb 255;
;@存储两个输入的参数
num1: resb 255;
num1_len: resb 255;
num2: resb 255;
num2_len: resb 255;
result: resb 255;
multiple: resb 255;
num_of_negative: resb 255;
negative: resb 255;
isFirst: resb 8
section .text
	global _start

_start:
	;@作者：rubisco
	;@时间：2021-10
	mov byte[negative],'-'
	mov byte[isFirst],0
	mov eax,msg
	call printStr
	;----提示符输出
	mov ecx,input
	call getInput
	call getNegative
	;@test5:检查getNegative是否符合期望
	;mov al,byte[num_of_negative]
	;cbw
  ;cwd
	;call printInt
	;@end test
	;----接收输入
	;@test1:检查输入是否正确
	;mov eax,input
	;call printStr
	;@end test
	call getNumbers
	;@test2:检查spilt是否正确
	;mov eax,num1
	;call printStr
	;mov eax,num2
	;call printStr
	;@end test
	;@test3:检查数字长度和输出整数功能是否正常
	;mov eax,dword[num1_len]
	;sub eax,dword[num2_len]
	;call printInt
	;@end test
	call bigAdd
	mov eax,result
	call format_result
	call printNegative_add
	call printResStr
mmul:
	mov eax,line
	call printStr
	;-----大数加法
	call bigMul
	mov eax,multiple
	call format_result
	call printNegative_mul
	call printResStr
	mov eax,line
	call printStr
	;-----exit------
	mov    ebx, 0
  mov    eax, 1
  int    80h
  ret



getInput:
;------------------------
;@用于接收用户输入,edx=缓冲区大小，ebx=想写入的文件
;@入口参数：ecx=变量的地址
;@出口参数：无
;------------------------
	push edx
	push ebx
	push eax
	mov edx,255
	mov ebx,0
	mov eax,3
	int 80h
	pop eax
	pop ebx
	pop edx
	ret

	
printStr:
;------------------------
;@控制台输出一个字符串的函数
;@输入:eax=字符串的地址
;@输出:控制台输出
;------------------------
	push ecx
	push ebx
	push edx
	mov ecx,eax
	call strLen
	mov ebx,1;标准输出
	mov eax,4
	int 80h
	pop edx
	pop ebx
	pop ecx
	ret
printInt:
;------------------------
;@控制台输出一个数字
;@输入：eax=要输出的数字
;@输出：控制台输出
;------------------------
    push   eax
    push   ecx
    push   edx
    push   esi
    mov    ecx, 0
  divideLoop:
    inc    ecx       ;存有位数
    mov    edx, 0
    mov    esi, 10
    idiv   esi       ;eax=eax/10
    add    edx, 48   ;edx存有余数
    push   edx       ;edx存入栈
    cmp    eax, 0
    jnz    divideLoop
  printLoop:
    dec    ecx
    mov    eax, esp
    call   printStr
    pop    eax
    cmp    ecx, 0
    jnz    printLoop
    pop    esi
    pop    edx
    pop    ecx
    pop    eax
    ret

strLen:
;------------------------
;@计算字符串长度的函数
;@输入:eax=字符串的地址
;@输出:edx=输出字符串的长度
;------------------------
	push ebx
	push eax
	mov ebx,eax ;将eax、ebx赋值为同一个地址
  nextChar:
  	cmp byte[eax],0
  	jz finished
  	inc eax
  	jmp nextChar
  finished:
  	sub eax,ebx
  	mov edx,eax
  	pop eax
  	pop ebx
  	ret

getNumbers:
;------------------------
;@将带一个空格的两个数字准确分开
;@输入input
;@改变num1和num2
;------------------------
	pushad;保护寄存器
	mov eax,input
	mov ebx,eax
	mov ecx,num1
	mov edx,num2;用ecx、edx分别保存num1、num2
	mov esi,num1_len
	mov edi,num2_len
	mov byte[esi],0
	mov byte[edi],0
  findSpace:
  	cmp byte[eax],20h
  	je getSpace
  	mov bl,byte[eax]
  	mov byte[ecx],bl
  	inc eax
  	inc ecx
  	inc byte[esi]
  	jmp findSpace
  getSpace:
  	inc eax;此处为空格
  secondNum:
  	cmp byte[eax],0
  	jz finish
  	mov bl,byte[eax]
  	mov byte[edx],bl
  	inc eax
  	inc edx
  	inc byte[edi]
  	jmp secondNum
  finish:
  	dec byte[edi]
  	popad
  	ret

bigAdd:
;------------------------
;@将两个数字相加
;@输入num1，num2
;@输出改变result
;------------------------
	cmp byte[num_of_negative],1
	je bigSub
	push eax
	push ebx
	push edx
	mov ecx,result
	;将result地址赋给ecx，可以通过修改ecx的值来改变result
	
	mov eax,num1
	mov ebx,num2
	mov esi,dword[num1_len]
	mov edi,dword[num2_len]
	cmp esi,edi
	ja num1_longer
	je ture_add
	jmp num2_longer
  num1_longer:
  	sub esi,edi;此时esi保存长出来的长度
  num1_loop:
  	cmp esi,0
  	je ture_add
  	mov dl,byte[eax]
  	mov byte[ecx],dl
  	inc eax
  	inc ecx
  	dec esi
  	jmp num1_loop
  num2_longer:
  	sub edi,esi;
  num2_loop:
  	cmp edi,0
  	je ture_add
  	mov dl,byte[ebx]
  	mov byte[ecx],dl
  	inc ebx
  	inc ecx
  	dec edi
  	jmp num2_loop
  ture_add:
  	cmp byte[eax],0
  	je finish_add
  	mov dl,byte[eax]
  	add dl,byte[ebx]
  	sub dl,30h;减去ascii中对应的0
  	mov byte[ecx],dl
  	inc eax
  	inc ebx
  	inc ecx
  	jmp ture_add
  finish_add:
		pop edx
		pop ebx
		pop eax
		ret


format_result:
;------------------------
;@将数字变成10进制的数字形式
;@输入eax=数字的地址
;@输出eax=改变后数字的地址，注如果发生进位，会提前输出一个1
;------------------------
		push edx
		mov ebx,eax
		;dec ebx;数字从这里开始
	toEnd:
		cmp byte[eax],0
		je formatLoop
		inc eax
		jmp toEnd
	formatLoop:
		dec eax
		cmp eax,ebx
		je formatFinish
	formatLoop_1:
		cmp byte[eax],'9'
		jna formatLoop
		mov edx,eax
		dec edx
		sub byte[eax],10
		add byte[edx],1
		jmp formatLoop_1
	formatFinish:
		mov ecx,0
		cmp byte[eax],'9'
		ja extraPrint
		pop edx
		ret
	extraPrint:
		inc ecx
		sub byte[eax],10
		cmp byte[eax],'9'
		ja extraPrint
		push eax
		mov eax,ecx
		call printInt
		pop eax
		jmp formatFinish

bigMul:
;------------------------
;@将两个数字相乘
;@输入num1，num2
;@输出改变multiple
;------------------------
    pushad
    mov ecx,multiple;通过更改ecx来更改multiple
    mov edx,num1
    mov ebx,num2
    mov esi,0
    mov edi,0
    ;移植需要实现将ecx放置在末尾
    mov eax,0
    add eax,dword[num1_len]
    add eax,dword[num2_len]
  init:
    cmp eax,0
    je toEnd_num1
    mov byte[ecx],'0'
    dec eax
    inc ecx
    jmp init
  toEnd_num1:
    cmp byte[edx+1],0
    je toEnd_num2
    inc edx
    jmp toEnd_num1
  toEnd_num2:
    cmp byte[ebx+2],0
    je multi
    inc ebx
    jmp toEnd_num2
  multi:
    push ebx
    ;初始时edx应该指向num1的末位
    
    ;test4:查验edx、ebx是否正确指向预计位置
    ;mov eax,0
  	;mov al,byte[ebx]
    ;cbw
    ;cwd
    ;call printInt
    ;finish test
    mov eax,0
    cmp edx,num1
    jb finish_multi
    mov al,byte[edx]
    
    sub al,30h
    jmp mul_loop
  mark:
    dec edx
    inc esi
    jmp multi
  mul_loop:
    
    cmp ebx,num2
    jb finish_mulLoop
    mov ah,byte[ebx]
    
    push edx
    mov dl,al
    sub ah,30h
    mul ah
    push ecx
    sub ecx,esi
    sub ecx,edi
    sub ecx,1
    cmp byte[ecx],150
    ja simple_format_byte
  finish_simple_format_byte:
    add byte[ecx],al
    pop ecx
    mov al,dl
    pop edx
    dec ebx
    inc edi
    jmp mul_loop
  finish_mulLoop:
    mov edi,0
    pop ebx
    jmp mark
  finish_multi:
    pop ebx
    popad
    ret
  simple_format_byte:
  	sub byte[ecx],100
  	add byte[ecx-2],1
  	jmp finish_simple_format_byte


printResStr:
;------------------------
;@用于结果输出
;------------------------
		cmp byte[eax+1],0
		je printStr
		cmp byte[eax+1],'0'
		jb printStr
		cmp byte[eax+1],'9'
		ja printStr
		cmp byte[eax],'0'
		ja printStr
		inc eax
		jmp printResStr


printNegative_add:
;------------------------
;@用于输出一个负号
;------------------------
		pushad
		cmp byte[num_of_negative],1
		jna noprint
		mov eax,negative
		call printStr
	noprint:
		popad
		ret


printNegative_mul:
;------------------------
;@用于输出一个负号
;------------------------
		pushad
		cmp byte[num_of_negative],1
		jne noprint
		mov eax,negative
		call printStr
		popad
		ret


getNegative:
;------------------------
;@用于计算负数的个数
;@输入：无（使用变量input）
;@输出：num_of_negative
;------------------------
		pushad
		mov byte[num_of_negative],0
		mov eax,input
	N_loop:
		cmp byte[eax],'-'
		je isNegative
		cmp byte[eax],0
		je F_loop
		cmp byte[eax],' '
		je setIsFirst
		inc eax
		jmp N_loop
	isNegative:
		add byte[num_of_negative],1
		mov byte[eax],'0'
		inc eax
		jmp N_loop
	setIsFirst:
		cmp byte[num_of_negative],0
		ja setTrue
		inc eax
		jmp N_loop
	setTrue:
		mov byte[isFirst],1
		inc eax
		jmp N_loop
	F_loop:
		popad
		ret
	


bigSub:
;------------------------
;@将两个数字相减
;@输入num1，num2
;@输出改变result
;------------------------
	push eax
	push ebx
	push edx
	mov ecx,result
	;将result地址赋给ecx，可以通过修改ecx的值来改变result
	mov eax,num1
	mov ebx,num2
	mov esi,dword[num1_len]
	mov edi,dword[num2_len]
	cmp byte[isFirst],1
	je subLen1
	jb subLen2
	f_decLen:
	cmp esi,edi
	ja sub_num1_longer
	je same
	jmp sub_num2_longer
  sub_num1_longer:
  	sub esi,edi;此时esi保存长出来的长度
  	cmp byte[isFirst],1
  	je print1_negative
  sub_num1_loop:
  	cmp esi,0
  	je ture_sub
  	mov dl,byte[eax]
  	mov byte[ecx],dl
  	inc eax
  	inc ecx
  	dec esi
  	jmp sub_num1_loop
  sub_num2_longer:
  	sub edi,esi;
  	cmp byte[isFirst],0
  	je print2_negative
  sub_num2_loop:
  	cmp edi,0
  	je ture_sub_2
  	mov dl,byte[ebx]
  	mov byte[ecx],dl
  	inc ebx
  	inc ecx
  	dec edi
  	jmp sub_num2_loop
  same:
    push esi
    push edi
    push edx
    mov esi,eax
    mov edi,ebx
  same_loop:
    cmp byte[esi],0
    je finish_sub_0
    mov dl,byte[edi]
    cmp byte[esi],dl
    ja num1_bigger
    jb num2_bigger
    inc esi
    inc edi
    jmp same_loop
  finish_same:
    pop edx
    pop edi
    pop esi
    jmp ture_sub
  ture_sub_2:
    push esi
    mov esi,eax
    mov eax,ebx
    mov ebx,esi
    pop esi
  ture_sub:
  	cmp byte[eax],'0'
  	jb finish_sub
  	cmp byte[eax],'9'
  	ja finish_sub
  	mov dl,byte[eax]
  	
  	add dl,30h;
  	sub dl,byte[ebx]
  	
  	mov byte[ecx],dl
  	inc eax
  	inc ebx
  	inc ecx
  	jmp ture_sub
  finish_sub_0:
    mov eax,0
    call printInt
    pop edx
		pop ebx
		pop eax
    jmp mmul
  finish_sub:

		pop edx
		pop ebx
		pop eax
		mov eax,result
		call format_result_sub
		ret
	print1_negative:
		push eax
		mov eax,negative
		call printStr
		pop eax
		jmp sub_num1_loop
	print2_negative:
		push eax
		mov eax,negative
		call printStr
		pop eax
		jmp sub_num2_loop
	subLen1:
		dec esi
    inc eax
		jmp f_decLen
	subLen2:
		dec edi
    inc ebx
		jmp f_decLen
print_bigger_negative:
		push eax
		mov eax,negative
		call printStr
		pop eax
		jmp finish_same
num1_bigger:
    cmp byte[isFirst],1
    je print_bigger_negative
    jmp finish_same
num2_bigger:
    mov esi,eax
    mov eax,ebx
    mov ebx,esi
    cmp byte[isFirst],1
    jb print_bigger_negative
    jmp finish_same


format_result_sub:
;------------------------
;@将数字变成10进制的数字形式
;@输入eax=数字的地址
;@输出eax=改变后数字的地址，注如果发生进位，会提前输出一个1
;------------------------
		push edx
		mov ebx,eax
		;dec ebx;数字从这里开始
	stoEnd:
		cmp byte[eax],0
		je sformatLoop
		inc eax
		jmp stoEnd
	sformatLoop:
		dec eax
		cmp eax,ebx
		je sformatFinish
	sformatLoop_1:
		cmp byte[eax],'0'
		jnb sformatLoop
		mov edx,eax
		dec edx
		add byte[eax],10
		sub byte[edx],1
		jmp sformatLoop_1
	sformatFinish:
		mov ecx,0
		pop edx
		ret
