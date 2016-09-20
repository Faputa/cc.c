/*
 * 方案一:
 * 1.先压ip、bp后压args
 * 2.push新局部变量
 */
栈帧设计：
|   ···   |bottom
│ hide_ip │
│ hide_bp │
│  arg_1  │<- bp
│   ···   │
│  arg_n  │
│  var_1  │<- sp
│   ···   │
│  var_n  │
│  opr_1  │
│   ···   │top

代码设计：
╭caller──────────╮
│ ···            │
│ push out_ip    │
│ push bp        │
│ push arg_1     │
│ ···            │
│ push arg_n     │
│ ip = callee    │
│ ╭callee──────╮ │
│ │ bp = sp    │ │
│ │ bp -= argc │ │bp指向arg_1
│ │ ···        │ │
│ │ [push var] │ │push新变量
│ │ ···        │ │
│ │ sp = bp    │ │
│ │ pop bp     │ │
│ │ pop ip     │ │
│ ╰────────────╯ │
│ ···            │<- out_ip
╰────────────────╯

/*
 * 方案二:
 * 1.先压ip、bp后压args
 * 2.sp直接指向局部变量空间之后以设置局部变量空间
 * 3.此方式易于设置数组和结构体
 */
栈帧设计：
|   ···   |bottom
│ hide_ip │
│ hide_bp │
│  arg_1  │<- bp
│   ···   │
│  arg_n  │
│  var_1  │
│   ···   │
│  var_n  │
│  opr_1  │<- sp
│   ···   │top

代码设计：
╭caller──────────╮
│ ···            │
│ push out_ip    │
│ push bp        │
│ push arg_1     │
│ ···            │
│ push arg_n     │
│ ip = callee    │
│ ╭callee──────╮ │
│ │ bp = sp    │ │
│ │ bp -= argc │ │bp指向arg_1
│ │ sp += varc │ │sp指向局部变量空间之后
│ │ ···        │ │
│ │ [ass var]  │ │赋值新变量
│ │ ···        │ │
│ │ sp = bp    │ │
│ │ pop bp     │ │
│ │ pop ip     │ │
│ ╰────────────╯ │
│ ···            │<- out_ip
╰────────────────╯

/*
 * 方案三:
 * 1.先压入后压入ip、bp
 * 2.不调整bp：参数向前偏移寻址，局部变量向后偏移寻址，从而避免局部变量寻址时因参数个数不匹配而引发的异常
 * 3.push新局部变量
 */
栈帧设计：
|   ···   |bottom
│  arg_1  │
│   ···   │
│  arg_n  │
│ hide_ip │
│ hide_bp │
│  var_1  │<- bp、sp
│   ···   │
│  var_n  │
│  opr_1  │
│   ···   │top

代码设计：
╭caller──────────╮
│ ···            │
│ push arg_1     │
│ ···            │
│ push arg_n     │
│ push out_ip    │
│ ip = callee    │
│ ╭callee──────╮ │
│ │ push bp    │ │
│ │ bp = sp    │ │
│ │ ···        │ │
│ │ [push var] │ │
│ │ ···        │ │
│ │ sp = bp    │ │防止语句块中return不能保证完全pop var从而无法正确pop ip、bp
│ │ pop bp     │ │
│ │ pop ip     │ │
│ ╰────────────╯ │
│ sp -= argc     │<- out_ip │ 重置sp指向arg_1的位置
│ ···            │
╰────────────────╯

/*
 * 方案四:
 * 1.先压入后压入ip、bp
 * 2.不调整bp：参数先前寻址，局部变量向后寻址
 * 3.sp直接指向局部变量空间之后以设置局部变量空间
 * 4.此方式易于设置数组和结构体
 */
栈帧设计：
|   ···   |bottom
│  arg_1  │
│   ···   │
│  arg_n  │
│ hide_ip │
│ hide_bp │
│  var_1  │<- bp
│   ···   │
│  var_n  │
│  opr_1  │<- sp
│   ···   │top

代码设计：
╭caller──────────╮
│ ···            │
│ push arg_1     │
│ ···            │
│ push arg_n     │
│ push out_ip    │
│ ip = callee    │
│ ╭callee──────╮ │
│ │ push bp    │ │
│ │ bp = sp    │ │
│ │ sp += varc │ │sp指向局部变量空间之后
│ │ ···        │ │
│ │ [ass var]  │ │赋值新变量
│ │ ···        │ │
│ │ sp = bp    │ │
│ │ pop bp     │ │
│ │ pop ip     │ │
│ ╰────────────╯ │
│ sp -= argc     │<- out_ip │ 重置sp指向arg_1的位置
│ ···            │
╰────────────────╯
或
╭caller──────────╮
│ ···            │
│ push arg_1     │
│ ···            │
│ push arg_n     │
│ push sp        │
│ push out_ip    │
│ ip = callee    │
│ ╭callee──────╮ │
│ │ push bp    │ │
│ │ bp = sp    │ │
│ │ sp += varc │ │sp指向局部变量空间之后
│ │ ···        │ │
│ │ [ass var]  │ │赋值新变量
│ │ ···        │ │
│ │ sp = bp    │ │
│ │ pop bp     │ │
│ │ pop ip     │ │
│ ╰────────────╯ │
│ pop sp         │<- out_ip │ 重置sp指向arg_1的位置
│ ···            │
╰────────────────╯

/*
 * 方案五:
 * 1.先压入后压入ip、bp
 * 2.不调整bp：参数先前寻址，局部变量向后寻址
 * 3.sp直接指向局部变量空间之后以设置局部变量空间
 * 4.参数从右向左反序压栈
 * 5.此方式易于设置数组和结构体，易于实现变长参数
 */
栈帧设计：
|   ···   |bottom
│  arg_n  │
│   ···   │
│  arg_1  │
│ hide_sp │
│ hide_ip │
│ hide_bp │
│  var_1  │<- bp
│   ···   │
│  var_n  │
│  opr_1  │<- sp
│   ···   │top

代码设计：
╭caller──────────╮
│ ···            │
│ push arg_1     │
│ ···            │
│ push arg_n     │
│ push sp        │
│ push out_ip    │
│ ip = callee    │
│ ╭callee──────╮ │
│ │ push bp    │ │
│ │ bp = sp    │ │
│ │ sp += varc │ │sp指向局部变量空间之后
│ │ ···        │ │
│ │ [ass var]  │ │赋值新变量
│ │ ···        │ │
│ │ sp = bp    │ │
│ │ pop bp     │ │
│ │ pop ip     │ │
│ ╰────────────╯ │
│ pop sp         │<- out_ip │ 重置sp指向arg_1的位置
│ ···            │
╰────────────────╯

/*
 * 方案X（有问题、废弃）:
 * 1.先压args后压ip、bp
 * 2.push新局部变量
 * 问题：语句块中return时不能保证完全pop var从而无法正确pop ip、bp
 */
栈帧设计：
|   ···   |bottom
│  arg_1  │<- bp
│   ···   │
│  arg_n  │
│ hide_ip │
│ hide_bp │
│  var_1  │<- sp
│   ···   │
│  var_n  │
│  opr_1  │
│   ···   │top

代码设计：
╭caller──────────────╮
│ ···                │
│ push arg_1         │
│ ···                │
│ push arg_n         │
│ push out_ip        │
│ ip = callee        │
│ ╭callee──────────╮ │
│ │ push bp        │ │
│ │ bp = sp        │ │
│ │ bp -= argc + 2 │ │
│ │ ···            │ │
│ │ [push var]     │ │
│ │ ···            │ │
│ │ pop bp         │ │
│ │ pop ip         │ │
│ ╰────────────────╯ │
│ sp -= argc         │
│ ···                │<- out_ip
╰────────────────────╯
