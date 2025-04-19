input IN1 , IN2
input Clk
output OUT
wire op_f1,op_buf
FF1 (clk,q->IN1,d->op_f1)
FF1 (clk,q->IN2,d->op_f2)
BUFFER (a->op_F1,zn->op_buf)
NAND (a->op_F1,b->op_buf,zh->OUT)

