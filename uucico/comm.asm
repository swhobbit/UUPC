	TITLE	COMM
	PAGE	83,132
;	$Id: comm.asm 1.7 1993/05/31 01:39:23 ahd Exp $
;
;	$Log: comm.asm $
;; Revision 1.7  1993/05/31  01:39:23  ahd
;; Add return to spin loop (fix by Bill Plummer)
;; Drop FIFO size to 8
;;
;; Revision 1.4  1993/05/30  00:20:02  ahd
;; Insert minor delay to allow slow modems to catch up
;;
;; Revision 1.2  1992/12/18  12:08:25  ahd
;; Add Plummer's fix for bad TASM assemble of com_errors
;;
;
;  9-Oct-93 Gumpertz	Further improved RTI and made it exit via LSI2 when
;			it still has the LSR available for counting errors.
;			Added/removed some JMP $+2 instructions to get some
;			I/O delay for slow 8259 and UART (8250) chips in
;			some old machines; I think one JMP is plenty for an
;			80286; the 386 and 486 are unlikely to be found in
;			machines that use old chips like the 8250.  Moby Sigh.
;			I suppose I will need some volunteers to test it.
;			Simplified CHROUT and then expanded it inline and
;			deleted the subroutine.  Simplified SENDII.
;  8-Oct-93 Gumpertz	Added _rts_off and _rts_on to support hardware
;			handshaking with modems.
;			Changed RXI to read one character without testing LSR.
;  7-Oct-93 Gumpertz	Made a number of changes to improve execution speed:
;			Changed START_TDATA, END_TDATA, START_RDATA, END_RDATA
;			to DWORDS.  Deleted TBuff and RBuff; (START_TDATA+2):0
;			and (START_RDATA+2):0 are now used instead.
;			Rearranged SPTAB for word alignment.
;			Changed SEND_OK to not be 0/1 but 0/10H/20H/30H
;			Deleted BP saving, setting, and restoring in routines
;			that don't use BP.
;			Added checks for R_SIZE or S_SIZE not powers of 2.
;			Changed receive_com and sen_com to NOT use CLI
;			unless really needed.
;			Expanded TX_CHR inline and deleted the subroutine.
;			Changed transmit FIFO back to 16 bytes and added
;			IER_SHADOW to save reading the IER.
;			Added a few even pseudo-ops togain a bit of speed.
; 28-Jun-93 Gumpertz	Added COM3IRQ5 option (and a few minor cleanups)
; 14-Jun-93 plummer	Add RET to spinloop routine
; 14-Jun-93 plummer	Set FIFO thresholds to 8 rather than 16 bytes
; 18-May-93 plummer	Define IO$DELAY and use in UART type determination
; 16-May-93 plummer	Debug code to printout UART type
; 22-Apr-93 plummer	Make case consistent in "TBuff" so it links properly
;  2-Dec-92 plummer	Fix com_errors() again.  Change got lost.
; Fix com_errors() to avoid problems with tasm.  Plummer, 11/16/92
; 8259 EOI issued after interrupts serviced.  Plummer, 3/25/92
; Fix botch in Set_Baud.  Plummer, 3/20/92
; Put in Gordon Lee's cure from dropped interrupts.  Plummer, 3/19/92
; TEMPORARY ioctl_com().  Plummer, 3/9/92.
; Clear OUT2 bit in UART.  Some machines use it so enable IRQ. Plummer, 3/9/92
; Release send buffer if we can't assign a recv buffer.  Plummer, 3/9/92
; Move EQU's outside of SP_TAB struc definition.  ahd, 3/8/92.
; ahd changes: short jmp's out of range in INST, OPEN ???  (ahd, 3/?/92)
; open_com() leaves DTR unchanged so Drew's autobaud works.  Plummer, 3/2/92
; Missing DX load in close_com() -- FIFO mode not cleared.  Plummer, 3/2/92
; C calling convention does not require saving AX, BX, CX, DX. Plummer 2/23/92
; Flush consideration of the PC Jr.  Wm. W. Plummer, 2/15/92
; Cleanup PUSHF/POPF and CLI/STI usage.  Wm. W. Plummer, 2/15/92
; Make SENDII have Giles Todd's change.  Wm. W. Plummer, 2/15/92
; Changes to Giles Todd's code to support dynamic buffers.  Plummer, 2/3/92
; 26 Jan 92 Giles Todd	Prime THR for UARTs which do not give a Tx empty
;			interrupt when Tx interrupts are enabled.
; S_SIZE & R_SIZE may be set with -D to MASM.  Wm. W. Plummer, 1/19/92
; Assign buffers dynamically.  Wm. W. Plummer, 1/19/92
; Unfix byte length -- I screwed up.  Wm. W. Plummer, 1/15/92
; Fix byte length with specific PARITY select.	Wm. W. Plummer, 1/13/92
; Buffers up to 4096 per AHD.  Wm. W. Plummer, 1/1/92
; Always use FIFO length of 16 on send side.  Wm. W. Plummer, 12/30/91.
; Init DSR and CTS previous state from current status.	Wm. Plummer, 12/30/91.
; UUPC conditional to disable v.24.  Wm. W. Plummer, 12/30/91.
; Buffer sizes up to 2048 per ahd.  Wm. W. Plummer, 12/15/91.
; dtr_on() switches to D connection if CTS&DSR don't come up.  WWP, 12/15/91.
; New dtr_on() logic.  Wm. W. Plummer, 12/11/91
; Fix bad reg. per report from user.  Wm. W. Plummer, 12/11/91
; Semicolon before control-L's for MASM 5.00 per ahd. Wm. W. Plummer, 12/8/91
; Use AHD's handling of COM ports.  Wm. W. Plummer, 11/29/91
; Buffer sizes reduced and required to be 2**N.  Wm. W. Plummer, 11/11/91
; Accomodate V.24 requirements on DTR flaps.  Wm. W. Plummer 10/15/91
; Revised DTR_ON_COM to solve user problem.  Wm. W. Plummer, 10/3/91
; Make time delays independent of CPU speed.  Wm. W. Plummer, 9/16/91
; Use interrupts to trace CD, DSR, Wm. W. Plummer, 9/16/91
; Remove modem control from TXI. Wm. W. Plummer, 9/13/91
; Completely redo the XOFF/XON logic.  Too many races before. Wm. W. Plummer
; Revise interrupt dispatch for speed & function.  William W. Plummer, 9/12/91
; Merge in ahd's changes to flush control Q,S when received as flow control
; SEND buffer allows one byte for a SENDII call.  Avoids flow control
;  lockups. - William W. Plummer, 8/30/91
; Support for NS16550A chip with SILO - William W. Plummer, 8/30/91
; Add modem_status() routine - William W. Plummer, 7/2/91
; Put wrong code under AHD conditional - William W. Plummer, 7/2/91
; Change TITLE, repair bad instr after INST3 - William W. Plummer, 7/1/91
; Modified to use COM1 thru COM4 - William W. Plummer, 2/21/91
; Eliminate (incomplete) support for DOS1 - William W. Plummer, 11/13/90

; Changes may be copied and modified with no notice.  Copyrights and copylefts
; are consider silly and do not apply.	--  William W. Plummer

; modified to use MSC calling sequence.  jrr 3/86
;****************************************************************************
; Communications Package for the IBM PC, XT, AT and strict compatibles.
; May be copied and used freely -- This is a public domain program
; Developed by Richard Gillmann, John Romkey, Jerry Saltzer,
; Craig Milo Rogers, Dave Mitton and Larry Afrin.
;
; We'd sure like to see any improvements you might make.
; Please send all comments and queries about this package
; to GILLMANN@USC-ISIB.ARPA
;
; o Supports both serial ports simultaneously
; o All speeds to 19200 baud
; o Compatible with PC, XT, AT
; o Built in XON/XOFF flow control option
; o C language calling conventions
; o Logs all comm errors
; o Direct connect or modem protocol
	PAGE;
;
; Buffer sizes -- *** MUST be powers of 2 ****

IFDEF UUPC
	R_SIZE	EQU	4096
	S_SIZE	EQU	4096
ENDIF

; If not set above, maybe on assembler command line.  But if not, ...
IFNDEF R_SIZE
	R_SIZE	EQU	512	; Recv buffer size
ENDIF
IFNDEF S_SIZE
	S_SIZE	EQU	512	; Send buffer size
ENDIF

IF (R_SIZE AND -R_SIZE) NE R_SIZE
	.ERR	R_SIZE must be a power of 2
ENDIF

IF (S_SIZE AND -S_SIZE) NE S_SIZE
	.ERR	S_SIZE must be a power of 2
ENDIF

; INTERRUPTS
VECIRQ3 EQU	08H+3		; IRQ3 VECTOR FROM 8259
VECIRQ4 EQU	08H+4		; IRQ4 VECTOR FROM 8259
VECIRQ5 EQU	08H+5		; IRQ5 VECTOR FROM 8259

IRQ3	EQU	2*2*2		; 8259A OCW1 MASK, M3=1, A0=0
IRQ4	EQU	2*2*2*2		; 8259A OCW1 MASK, M4=1, A0=0
IRQ5	EQU	2*2*2*2*2	; 8259A OCW1 MASK, M4=1, A0=0

NIRQ3	EQU	NOT IRQ3 AND 0FFH ; COMPLEMENT OF ABOVE
NIRQ4	EQU	NOT IRQ4 AND 0FFH ; COMPLEMENT OF ABOVE
NIRQ5	EQU	NOT IRQ5 AND 0FFH ; COMPLEMENT OF ABOVE

EOI3	EQU	3 OR 01100000B	; 8259A OCW2 SPECIFIC IRQ3 EOI, A0=0
EOI4	EQU	4 OR 01100000B	; 8259A OCW2 SPECIFIC IRQ4 EOI, A0=0
EOI5	EQU	5 OR 01100000B	; 8259A OCW2 SPECIFIC IRQ5 EOI, A0=0

; 8259 PORTS
INTA00	EQU	20H		; 8259A PORT, A0 = 0
INTA01	EQU	21H		; 8259A PORT, A0 = 1

; FLOW CONTROL CHARACTERS
CONTROL_Q EQU	11H		; XON
CONTROL_S EQU	13H		; XOFF
; MISC.
DOS	EQU	21H		; DOS FUNCTION CALLS

;
; ROM BIOS Data Area
;
RBDA	SEGMENT AT 40H
RS232_BASE DW	4 DUP(?)	; ADDRESSES OF RS232 ADAPTERS
RBDA	ENDS

PAGE;
;
; TABLE FOR EACH SERIAL PORT
;
SP_TAB		STRUC
PORT		DB	?	; 1 OR 2 OR 3 OR 4
INSTALLED	DB	?	; IS PORT INSTALLED ON THIS PC? (1=YES,0=NO)
; PARAMETERS FOR THIS INTERRUPT LEVEL
IRQVEC		DB	?	; INTERRUPT NUMBER
IRQ		DB	?	; 8259A OCW1 MASK
NIRQ		DB	?	; COMPLEMENT OF ABOVE
EOI		DB	?	; 8259A OCW2 SPECIFIC END OF INTERRUPT
; INTERRUPT HANDLERS FOR THIS LEVEL
INT_HNDLR	DW	?	; OFFSET TO INTERRUPT HANDLER
OLD_COM		DD	?	; OLD HANDLER'S OFFSET AND SEGMENT
; ATTRIBUTES
BAUD_RATE	DW	?	; 19200 MAX (maybe 38400 or 57600 with 16550?)
CONNECTION	DB	?	; M(ODEM), D(IRECT)
PARITY		DB	?	; N(ONE), O(DD), E(VEN), S(PACE), M(ARK)
STOP_BITS	DB	?	; 1, 2
XON_XOFF	DB	?	; E(NABLED), D(ISABLED)
UART_SILO_LEN	DB	?	; Size of a transmit silo chunk (1 for 8250)
; FLOW CONTROL STATE
HOST_OFF	DB	?	; HOST XOFF'ED (1=YES,0=NO)
PC_OFF		DB	?	; PC XOFF'ED (1=YES,0=NO)
URGENT_SEND	DB	?	; We MUST send one byte (XON/XOFF)
SEND_OK		DB	?	; DSR and CTS bits, masked from MSR
IER_SHADOW	DB	?	; shadow copy of what we last wrote to IER
; ERROR COUNTS
ERROR_BLOCK	DW	8 DUP(?); EIGHT ERROR COUNTERS

; UART PORTS - DATREG thru MSR must be in order shown.
DATREG		DW	?	; DATA REGISTER
IER		DW	?	; INTERRUPT ENABLE REGISTER
IIR		DW	?	; INTERRUPT IDENTIFICATION REGISTER (RO)
LCR		DW	?	; LINE CONTROL REGISTER
MCR		DW	?	; MODEM CONTROL REGISTER
LSR		DW	?	; LINE STATUS REGISTER
MSR		DW	?	; MODEM STATUS REGISTER
;
; BUFFER POINTERS
START_TDATA	DD	?	; POINTER TO FIRST CHARACTER IN X-MIT BUFFER
END_TDATA	DD	?	; POINTER TO FIRST FREE SPACE IN X-MIT BUFFER
START_RDATA	DD	?	; POINTER TO FIRST CHARACTER IN REC. BUFFER
END_RDATA	DD	?	; POINTER TO FIRST FREE SPACE IN REC. BUFFER
; BUFFER COUNTS
SIZE_TDATA	DW	?	; NUMBER OF CHARACTERS IN X-MIT BUFFER
SIZE_RDATA	DW	?	; NUMBER OF CHARACTERS IN REC. BUFFER
; BUFFERS
SP_TAB		ENDS

; SP_TAB EQUATES
; WE HAVE TO USE THESE BECAUSE OF PROBLEMS WITH STRUC
EOVFLOW		EQU	ERROR_BLOCK	; BUFFER OVERFLOWS
EOVRUN		EQU	ERROR_BLOCK+2	; RECEIVE OVERRUNS
EBREAK		EQU	ERROR_BLOCK+4	; BREAK CHARS
EFRAME		EQU	ERROR_BLOCK+6	; FRAMING ERRORS
EPARITY		EQU	ERROR_BLOCK+8	; PARITY ERRORS
EXMIT		EQU	ERROR_BLOCK+10	; TRANSMISSION ERRORS
EDSR		EQU	ERROR_BLOCK+12	; DATA SET READY ERRORS
ECTS		EQU	ERROR_BLOCK+14	; CLEAR TO SEND ERRORS
DLL		EQU	DATREG		; LOW DIVISOR LATCH
DLH		EQU	IER		; HIGH DIVISOR LATCH

;
; Equates having to do with the FIFO
;
FCR		EQU	IIR	; FIFO Control Register (WO)
 ; Bits in FCR for NS16550A UART.  Note that writes to FCR are ignored
 ; by other chips.
 FIFO_ENABLE	EQU	001H	; Enable FIFO mode
 FIFO_CLR_RCV	EQU	002H	; Clear receive FIFO
 FIFO_CLR_XMT	EQU	004H	; Clear transmit FIFO
 FIFO_STR_DMA	EQU	008H	; Start DMA Mode
 ; 10H and 20H bits are register bank select on some UARTs (not handled)
 FIFO_SZ_1	EQU	000H	; RCV Warning level is 1 byte in the FIFO
 FIFO_SZ_4	EQU	040H	; RCV Warning level is 4 bytes in the FIFO
 FIFO_SZ_8	EQU	080H	; RCV Warning level is 8 bytes in the FIFO
 FIFO_SZ_14	EQU	0C0H	; RCV Warning level is 14 bytes in the FIFO
 ;
 ; Commands used in code to operate FIFO.  Made up as combinations of above
 ;
 FIFO_CLEAR	EQU	0	; Turn off FIFO
 FIFO_SETUP	EQU	FIFO_SZ_8 OR FIFO_ENABLE
 FIFO_INIT	EQU	FIFO_SETUP OR FIFO_CLR_RCV OR FIFO_CLR_XMT
 ;
 ; Miscellaneous FIFO-related stuff
 ;
FIFO_ENABLED	EQU	0C0H	; 16550 makes these equal FIFO_ENABLE
FIFO_LEN	EQU	16	; Length of the transmit FIFO in a 16550A
	PAGE;
;	put the data in the DGROUP segment
;	far calls enter with DS pointing to DGROUP
;
DGROUP	GROUP _DATA
_DATA	SEGMENT PUBLIC 'DATA'
;
; DATA AREAS FOR EACH PORT
AREA1	SP_TAB	<1,0,VECIRQ4,IRQ4,NIRQ4,EOI4,OFFSET COM_TEXT:INT_HNDLR1> ; COM1
AREA2	SP_TAB	<2,0,VECIRQ3,IRQ3,NIRQ3,EOI3,OFFSET COM_TEXT:INT_HNDLR2> ; COM2
 IFDEF COM3IRQ5 ; special KLUDGE: assemble for COM3 with IRQ5 instead of IRQ4
AREA3	SP_TAB	<3,0,VECIRQ5,IRQ5,NIRQ5,EOI5,OFFSET COM_TEXT:INT_HNDLR3> ; COM3
 ELSE		; normal: assemble for COM3 with IRQ4
AREA3	SP_TAB	<3,0,VECIRQ4,IRQ4,NIRQ4,EOI4,OFFSET COM_TEXT:INT_HNDLR3> ; COM3
 ENDIF
AREA4	SP_TAB	<4,0,VECIRQ3,IRQ3,NIRQ3,EOI3,OFFSET COM_TEXT:INT_HNDLR4> ; COM4
CURRENT_AREA	DW OFFSET DGROUP:AREA1	; CURRENTLY SELECTED AREA

DIV50		DW 2304			; ACTUAL DIVISOR FOR 50 BAUD IN USE

IFDEF DEBUG
 ST8250		DB "8250 or 16450$"
 ST16550	DB "16550AN$"
 STUART		DB " UART detected", 0DH, 0AH, '$'
ENDIF

_DATA	ENDS




COM_TEXT SEGMENT PARA PUBLIC 'CODE'
	 ASSUME CS:COM_TEXT,DS:DGROUP,ES:NOTHING

	 PUBLIC AREA1, AREA2, AREA3, AREA4
	 PUBLIC _select_port
	 PUBLIC _save_com
	 PUBLIC _install_com
	 PUBLIC _restore_com
	 PUBLIC _open_com
	 PUBLIC _ioctl_com
	 PUBLIC _close_com
	 PUBLIC _dtr_on
	 PUBLIC _dtr_off
	 PUBLIC _rts_off
	 PUBLIC _rts_on
	 PUBLIC _r_count
	 PUBLIC _s_count
	 PUBLIC _receive_com
	 PUBLIC _send_com
	 PUBLIC _sendi_com
IFNDEF UUPC
	 PUBLIC _send_local
ENDIF
	 PUBLIC _break_com
	 PUBLIC _com_errors
	 PUBLIC _modem_status
IFDEF DEBUG
	 PUBLIC INST2, INST4
	 PUBLIC OPEN1, OPEN2, OPENX
	 PUBLIC DTRON1, DTRON6, DTRONF, DTRDIR, DTRONS, DTRONX
	 PUBLIC RECV1, RECV3, RECV4, RECVX
	 PUBLIC SEND1, SEND2, SEND3, SENDX
	 PUBLIC WaitN, WaitN1, WaitN2
	 PUBLIC SENDII, SENDII1, SENDII2, SENDII4
	 PUBLIC SPINLOOP
	 PUBLIC BREAKX
	 PUBLIC INT_HNDLR1, INT_HNDLR2, INT_HNDLR3, INT_HNDLR4
	 PUBLIC INT_COMMON, REPOLL, INT_END
	 PUBLIC LSI, LSI2
	 PUBLIC MSI
	 PUBLIC TXI, TXI1, TXI3, TXI4, TXI9
	 PUBLIC RXI, RXI0, RXINXT, RXIFUL, RXI1, RXI2, RXI3
ENDIF
	PAGE;
; Notes, thoughts and explainations by Bill Plummer.  These are intended to
; help those of you who would like to make modifications.

; Here's the order of calls in UUPC.  The routines in COMM.ASM are called
; from ulib.c.

; First (when a line in system has been read?), ulib&openline calls
;	 select_port()		; Sets up CURRENT_AREA
; then,  save_com()		; Save INT vector
; then,  install_com()		; Init area, hook INT
; then,  open_com(&cmd, 'D', 'N', STOP*T, 'D')  ; Init UART, clr bufs
; then,  dtr_on().

; At that point the line is up and running.  UUPC calls ulib&sread()
; which calls,	receive_com();

; And UUPC calls ulib&swrite()
; which calls,	send_com();

; To cause an error that the receiver will see, UUPC calls ulib&ssendbrk();
; which calls,	break_com();

; When all done with the line, UUPC calls ulib&closeline()
; which calls,	dtr_off();
; then,  close_com();
; then,  restore_com(); 	; Unhook INT
; and,	 stat_errors();


; Note: On the PC COM1 and COM3 share IRQ4, while COM2 and COM4 share IRQ3.
; BUT, only one device on a given IRQ line can be active at a time.  So it is
; sufficient for UUPC to hook whatever IRQ INT its modem is on as long as it
; unhooks it when it is done with that COM port.  COMM cannot be an installed
; device driver since it must go away when UUPC is done so that other devices
; on the same INTs will come back to life.  Also, it is OK to have a static
; CURRENT_AREA containing the current area that UUPC is using.

; Note about using the NS16550A UART chip's FIFOs.  These are operated as
; silos.  In other words when an interrupt happens because the receive(send)
; FIFO is nearly full(empty), as many bytes as possible are transferred and
; the interrupt dismissed.  Thus, the interrupt load is lowered.


; Concerning the way the comm line is brought up.
; There are two basic cases, the Direct ('D') connection and the Modem ('M')
; connection.  For either UUPC calls dtr_on_com() to bring up the line.  This
; causes Data Terminal Ready (DTR) and Request To Send (RTS) to be set.  Note
; this is OK for a simple 3-wire link but may be REQUIRED for a COM port
; connected to an external modem.

; The difference between a D connection and an M connection is
; whether or not the PC can expect any signals back from the modem.  If
; there is a simple 3-wire link, Data Set Ready will be floating.
; (Actually, some wise people jumper Data Terminal Read back to Data
; Set Ready so the PC sees its own DTR appear as DSR.)	UUPC should be
; able to handle the simplest cable as a design feature.  So both D and
; M connections send out DTR and RTS, but only the M connection expects
; a modem to respond.

; Then, if it is full modem connection (M), we wait for a few
; seconds hoping that both Data Set Ready (DSR) and Clear To Send (CTS)
; will come up.  If they don't, the associated counters are incremented
; for subsequent printing in the error log.  Note that no error is
; reported from COMM to UUPC at this point, although this would be a
; good idea.  COMMFIFO.ASM forces the connection to be a D type and lets
; UUPC storm ahead with its output trying to
; establish a link, but the output is never sent due to one of the
; control signals being false.	UUPC could check the modem status using
; a call which has been installed just for this purpose.

; Note, if you are going to connect your PC running UUPC to,
; say, a mainframe and you need hardware flow control (i.e., RTS-CTS
; handshaking), use a Modem connection.  Using a simple 3-wire cable
; forbids hardware flow control and UUPC must be instructed to use a
; Direct connection.  Refer to comments in the SYSTEMS file on how to
; make this selection.

; References used in designing the revisions to COMM.ASM:
;	1.	The UNIX fas.c Driver code.
;	2.	SLIP8250.ASM from the Clarkson driver set.
;	3.	NS16550A data sheet and AN-491 from National Semiconductor.
;	4.	Bell System Data Communications, Technical Reference for
;		Data Set 103A, Interface Specification, February, 1967
;	5.	Network mail regarding V.24
;	6.	Joe Doupnik
	PAGE;
;
; void far select_port(int)
;	Arg is 1..4 and specifies which COM port is referenced by
;	all other calls in this package.
;
_select_port PROC FAR
	push bp
	mov bp,sp
	MOV	AX,[BP+6]		; get argument
	CMP	AX,1			; Port 1?
	 JE	SP1			; Yes
	CMP	AX,2			; Port 2?
	 MOV	BX,OFFSET DGROUP:AREA2	; SELECT COM2 DATA AREA
	 JE	SPX			; Yes
	CMP	AX,3			; Port 3?
	 MOV	BX,OFFSET DGROUP:AREA3	; SELECT COM3 DATA AREA
	 JE	SPX			; Yes
	CMP	AX,4			; Port 4?
	 MOV	BX,OFFSET DGROUP:AREA4	; SELECT COM4 DATA AREA
	 JE	SPX			; Yes
	INT 20H				; N.O.T.A. ????? Halt for debugging!
	; Assume port 1 if continued
SP1:	MOV	BX,OFFSET DGROUP:AREA1	; SELECT COM1 DATA AREA
SPX:	MOV	CURRENT_AREA,BX		; SET SELECTION IN MEMORY
	mov sp,bp
	pop bp
	RET
_select_port ENDP
	PAGE;
;
; void far save_com(void)
;	Save the interrupt vector of the selected COM port.
;	N.B. save_com() and restore_com() call MUST be properly nested
;
_save_com PROC FAR
	PUSH SI
	PUSH	ES			; SAVE EXTRA SEGMENT
	MOV	SI,CURRENT_AREA		; SI POINTS TO DATA AREA

; Save old interrupt vector
	MOV	AH,35H			; FETCH INTERRUPT VECTOR CONTENTS
	MOV	AL,IRQVEC[SI]		; INTERRUPT NUMBER
	INT	DOS			; DOS 2 FUNCTION
	MOV	WORD PTR OLD_COM[SI],BX	; SAVE ES:BX
	MOV	WORD PTR OLD_COM[SI+2],ES ; FOR LATER RESTORATION
	POP	ES			; RESTORE ES
	POP SI
	RET				; DONE
_save_com ENDP
	PAGE;
;
; int far install_com(void)
;
;	Install the selected COM port.
;	Returns:	0: Failure
;			1: Success
;
; SET UART PORTS FROM RS-232 BASE IN ROM BIOS DATA AREA
; INITIALIZE PORT CONSTANTS AND ERROR COUNTS
;
; Assign blocks of memory for transmit and receive buffers
;
_install_com PROC FAR
	PUSH SI
	PUSH ES
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	CMP	INSTALLED[SI],1 	; Is port installed on this machine?
	 JNE	INST1			; NO, CONTINUE
	 JMP	INST9			; ELSE JUMP IF ALREADY INSTALLED

; Assign memory for transmit and receive buffers

INST1:
	MOV BX,S_SIZE/16		; Send buffer size in paragraphs
	MOV AX,4800H			; Allocate memory
	INT DOS
	 JC INSTFAIL			; Give fail return
	MOV WORD PTR START_TDATA[SI],0
	MOV WORD PTR START_TDATA[SI+2],AX
	MOV WORD PTR END_TDATA[SI],0
	MOV WORD PTR END_TDATA[SI+2],AX

	MOV BX,R_SIZE/16		; Receive buffer size in paragraphs
	MOV AX,4800H			; Allocate memory
	INT DOS
	 JC INSTFREET			; jump on FAILURE
	MOV WORD PTR START_RDATA[SI],0
	MOV WORD PTR START_RDATA[SI+2],AX
	MOV WORD PTR END_RDATA[SI],0
	MOV WORD PTR END_RDATA[SI+2],AX

IFDEF DEBUG
	PUSH DI
	CLD				; Go up in memory
	XOR AX,AX			; A zero to store
	MOV ES,START_TDATA[SI]		; Transmit buffer location
	MOV DI,AX
	MOV CX,S_SIZE/2			; Size of buffer
	REP STOSW			; Clear entire buffer
	MOV ES,START_RDATA[SI]		; Receive buffer location
	MOV DI,AX
	MOV CX,R_SIZE/2			; Size of buffer
	REP STOSW			; Clear entire buffer
	POP DI
ENDIF
	PAGE;

	MOV	BX,RBDA 		; ROM BIOS DATA AREA
	MOV	ES,BX			; TO ES
	ASSUME	ES:RBDA

; Map port number (COMx) into IO Address using the RS232_Base[x] table in
; the BIOS data area.  If any of the ports is missing there should be a
; zero in the table for this COM port.	BIOS startup routines pack the table
; so that if you have a COM4 but no COM3, 2E8 will be found in 40:4 and 0
; will be in 40:6.

; N.B. The exact IO address in 40:x is irrelevant and may well be something
; other than the "standard" values if specially designed hardware is used.
; To minimize flack, we will use the standard value if the slot in the table
; is 0.  The bad side effect of this is that (in the standard losing case of
; a COM4 but no COM3) both COM3 and COM4 will reference the hardware at 2E8.

	CMP	PORT[SI],1		; PORT 1?
	 JE	INST3F8 		; Yes
	CMP	PORT[SI],2		; PORT 2?
	 JE	INST2F8 		; Yes
	CMP	PORT[SI],3		; PORT 3?
	 JE	INST3E8 		; Yes
	CMP	PORT[SI],4		; PORT 4?
	 JE	INST2E8 		; Yes
	INT	20H			; NOTA. (Caller is screwed up badly)

; We interrupt this program to bring you code that we want to be within
; short-jump range of the code that conditionally jumps to it:

INSTFREEBOTH:	; Unhand the receive and send buffers assigned above
	MOV ES,WORD PTR START_RDATA[SI+2] ; Receive buffer paragraph
	assume ES:nothing
	MOV AX,4900H			; Release memory
	INT DOS
	; Ignore error
INSTFREET:	; Unhand the send buffer assigned above
	MOV ES,WORD PTR START_TDATA[SI+2] ; Transmit buffer paragraph
	MOV AX,4900H			; Release memory
	INT DOS
	; Ignore error
	; Fall into INSTFAIL

; PORT NOT INSTALLED
INSTFAIL:
	XOR AX,AX
	JMP INSTX

	ASSUME	ES:RBDA
; And now back to your regularly scheduled program:

INST3F8:MOV AX,3F8H			; Standard COM1 location
	CMP	RS232_BASE+0,0000H	; We have information?
	 JE	INST2			; No --> Use default
	MOV	AX,RS232_BASE+0 	; Yes --> Use provided info
	JMP	SHORT INST2		; CONTINUE

INST2F8:MOV AX,2F8H			; Standard COM2 location
	CMP	RS232_BASE+2,0000H	; We have information?
	 JE	INST2			; No --> Use default
	MOV	AX,RS232_BASE+2 	; Yes --> Use provided info
	JMP	SHORT INST2		; CONTINUE

INST3E8:MOV AX,3E8H			; Standard COM3 location
	CMP	RS232_BASE+4,0000H	; We have information?
	 JE	INST2			; No --> Use default
	MOV	AX,RS232_BASE+4 	; Yes --> Use provided info
	JMP	SHORT INST2		; CONTINUE

INST2E8:MOV AX,2E8H			; Standard COM4 location
	CMP	RS232_BASE+6,0000H	; We have information?
	 JE	INST2			; No --> Use default
	MOV	AX,RS232_BASE+6 	; Yes --> Use provided info
	; Fall into INST2


; Now we have an IO address for the COMx that we want to use.  If it is
; anywhere in RS232_Base, we know that it has been check and is OK to use.
; So, even if my 2E8 (COM4) appears in 40:6 (normally for COM3), I can use
; it.

INST2:	CMP	AX,RS232_BASE		; INSTALLED?
	 JE	INST2A			; JUMP IF SO
	CMP	AX,RS232_BASE+2 	; INSTALLED?
	 JE	INST2A			; JUMP IF SO
	CMP	AX,RS232_BASE+4 	; INSTALLED?
	 JE	INST2A			; JUMP IF SO
	CMP	AX,RS232_BASE+6 	; INSTALLED?
	 JNE	INSTFREEBOTH 		; JUMP IF NOT
	; Fall into INST2A

INST2A: MOV	BX,DATREG		; OFFSET OF TABLE OF PORTS
	MOV	CX,7			; LOOP SIX TIMES
INST3:	MOV	WORD PTR [SI][BX],AX	; SET PORT ADDRESS
	INC	AX			; NEXT PORT
	ADD	BX,2			; NEXT WORD ADDRESS
	 LOOP	INST3			; RS232 BASE LOOP

; CLEAR ERROR COUNTS
	MOV	WORD PTR EOVFLOW[SI],0	; BUFFER OVERFLOWS
	MOV	WORD PTR EOVRUN[SI],0	; RECEIVE OVERRUNS
	MOV	WORD PTR EBREAK[SI],0	; BREAK CHARS
	MOV	WORD PTR EFRAME[SI],0	; FRAMING ERRORS
	MOV	WORD PTR EPARITY[SI],0	; PARITY ERRORS
	MOV	WORD PTR EXMIT[SI],0	; TRANSMISSION ERRORS
	MOV	WORD PTR EDSR[SI],0	; DATA SET READY ERRORS
	MOV	WORD PTR ECTS[SI],0	; CLEAR TO SEND ERRORS

	PUSHF				; Save caller's interrupt state
	CLI				; Stray interrupts cause havoc

	MOV DX,FCR[SI]			; Get FIFO Control Register
	MOV AL,FIFO_INIT
	OUT DX,AL			; Try to initialize the FIFO
	CALL SPINLOOP			; Permit I/O bus to settle
	MOV DX,IIR[SI]			; Get interrupt ID register
	IN AL,DX			; See how the UART responded
	AND AL,FIFO_ENABLED		; Keep only these bits
	MOV CL,1			; Assume size 1 for 8250/16450 case
	CMP AL,FIFO_ENABLED		; See if 16550A
	 JNE INST4			; Jump if not
	MOV CL,FIFO_LEN
INST4:	MOV UART_SILO_LEN[SI],CL	; Save chunk size for XMIT side
 IF FCR NE IIR				; FCR should be same as IIR
	MOV DX,FCR[SI]
 ENDIF
	MOV AL,FIFO_CLEAR		; disable the FIFOs until open_com
	OUT DX,AL

	POPF				; Restore caller's interrupt state

	MOV	AH,25H			; SET INTERRUPT VECTOR CONTENTS
	MOV	AL,IRQVEC[SI]		; INTERRUPT NUMBER
	MOV	DX,INT_HNDLR[SI]	; OUR INTERRUPT HANDLER [WWP]
	PUSH	DS			; SAVE DATA SEGMENT
	PUSH	CS			; COPY CS
	POP	DS			; TO DS
	INT	DOS			; DOS FUNCTION
	POP	DS			; RECOVER DATA SEGMENT

; PORT INSTALLED
INST9:	MOV AX,1
	;Fall into INSTX

; Common exit
INSTX:	MOV INSTALLED[SI],AL		; Indicate whether installed or not
IFDEF DEBUG
	MOV DX,OFFSET ST8250
	CMP UART_SILO_LEN[SI],1
	 JE INSTXX
	MOV DX,OFFSET ST16550
INSTXX: MOV AH,9
	INT DOS 			; Announce UART type
	MOV DX,OFFSET STUART
	INT DOS
ENDIF
	POP ES
	assume ES:nothing
	POP SI
	RET
_install_com ENDP
	PAGE;
;
; void far restore_com(void)
;	Restore original interrupt vector and release storage
;
_restore_com PROC FAR
	PUSHF
	PUSH SI
	PUSH ES
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	CLI
	MOV	INSTALLED[SI],0 	; PORT IS NO LONGER INSTALLED
	PUSH	DS			; SAVE DS
	LDS	DX,OLD_COM[SI]		; OLD INTERRUPT HANDLER IN DS:BX
	MOV	AH,25H			; SET INTERRUPT VECTOR FUNCTION
	MOV	AL,IRQVEC[SI]		; INTERRUPT NUMBER
	INT	DOS			; DOS FUNCTION
	POP	DS			; Recover our data segment

	MOV ES,WORD PTR START_TDATA[SI+2] ; Transmit buffer paragraph
	MOV AX,4900H			; Release memory
	INT DOS
	 ; Ignore error
	MOV ES,WORD PTR START_RDATA[SI+2] ; Receive buffer paragraph
	MOV AX,4900H
	INT DOS
	 ; Ignore error
	POP ES
	POP SI
	POPF
	RET
_restore_com ENDP
	PAGE;
;
; void far open_com(int Baud, char Conn, char Parity, char Stops, char Flow);
;
; CLEAR BUFFERS
; RE-INITIALIZE THE UART
; ENABLE INTERRUPTS ON THE 8259 INTERRUPT CONTROL CHIP
;
; [bp+6] = BAUD RATE
; [bp+8] = CONNECTION: M(ODEM), D(IRECT)
; [bp+10] = PARITY:	N(ONE), O(DD), E(VEN), S(PACE), M(ARK)
; [bp+12] = STOP BITS:	1, 2
; [bp+14] = XON/XOFF:	E(NABLED), D(ISABLED)
;
_open_com PROC FAR
	push bp
	mov bp,sp
	PUSH SI
	PUSHF
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	CLI				; INTERRUPTS OFF
	TEST INSTALLED[SI],1		; Port installed?
	 JNZ OPEN1			; Yes --> Proceed
	  JMP OPENX			; No  --> Get out

OPEN1:	mov ax,[bp+6]
	MOV	BAUD_RATE[SI],AX	; SET
	mov bh,[bp+8]
	MOV	CONNECTION[SI],BH	; ARGS
	mov bl,[bp+10]
	MOV	PARITY[SI],BL		; IN
	mov ch,[bp+12]
	MOV	STOP_BITS[SI],CH	; MEMORY
	mov cl,[bp+14]
	MOV	XON_XOFF[SI],CL

; RESET FLOW CONTROL
	MOV	HOST_OFF[SI],0		; HOST FLOWING
	MOV	PC_OFF[SI],0		; PC FLOWING
	MOV URGENT_SEND[SI],0		; No (high priority) flow ctl
	MOV SEND_OK[SI],0		; DTR&CTS are not on yet

; RESET BUFFER COUNTS AND POINTERS
	MOV	WORD PTR START_TDATA[SI],0
	MOV	WORD PTR END_TDATA[SI],0
	MOV	WORD PTR START_RDATA[SI],0
	MOV	WORD PTR END_RDATA[SI],0
	MOV	SIZE_TDATA[SI],0
	MOV	SIZE_RDATA[SI],0

;
; RESET THE UART
	MOV DX,MCR[SI]			; Modem Control Register
	IN AL,DX			; Get current settings
	JMP SHORT $+2
; In next line, comment doesn't match the constant.  Should it be 01H?  -RHG
	AND AL,0FEH			; Clr RTS, OUT1, OUT2 & LOOPBACK, but
	OUT DX,AL			; Not DTR (No hangup during autobaud)
	MOV DX,MSR[SI]			; Modem Status Register
	JMP SHORT $+2
	IN AL,DX			; Get current DSR and CTS states.
	JMP SHORT $+2
	AND AL,30H			; Init PREVIOUS STATE FLOPS to current
	OUT DX,AL			;  state and clear Loopback, etc.
	MOV SEND_OK[SI],AL		; If 30H, allow TXI to send out data
	JMP SHORT $+2
	IN AL,DX			; Re-read to get delta bits & clr int
	JMP SHORT $+2
OPEN2:	MOV DX,FCR[SI]			; I/O Address of FIFO control register
	MOV AL,FIFO_CLEAR		; Disable FIFOs
	OUT DX,AL			; Non-16550A chips will ignore this
	MOV	DX,LSR[SI]		; RESET LINE STATUS CONDITION
	JMP SHORT $+2
	IN	AL,DX
	MOV	DX,DATREG[SI]		; RESET RECEIVE DATA CONDITION
	JMP SHORT $+2
	IN	AL,DX
	MOV	DX,MSR[SI]		; RESET MODEM DELTAS AND CONDITIONS
	JMP SHORT $+2
	IN	AL,DX

	CALL Set_Baud			; Set the baud rate from arg
	PAGE;
; SET PARITY AND NUMBER OF STOP BITS
	MOV	AL,03H			; Default: NO PARITY + 8 bits data

	CMP	PARITY[SI],'O'          ; ODD PARITY REQUESTED?
	 JNE	P1			; JUMP IF NOT
	MOV	AL,0AH			; SELECT ODD PARITY + 7 bits data
	JMP	SHORT P4		; CONTINUE
;
P1:	CMP	PARITY[SI],'E'          ; EVEN PARITY REQUESTED?
	 JNE	P2			; JUMP IF NOT
	MOV	AL,1AH			; SELECT EVEN PARITY + 7 bits data
	JMP	SHORT P4		; CONTINUE
;
P2:	CMP	PARITY[SI],'M'          ; MARK PARITY REQUESTED?
	 JNE	P3			; JUMP IF NOT
	MOV	AL,2AH			; SELECT MARK PARITY + 7 bits data
	JMP SHORT P4

P3:	CMP PARITY[SI],'S'              ; SPACE parity requested?
	 JNE P4 			; No.  Must be 'N' (NONE)
	MOV AL,3AH			; Select SPACE PARITY + 7 bits data

P4:	TEST	STOP_BITS[SI],2 	; 2 STOP BITS REQUESTED?
	 JZ	STOP1			; NO
	OR	AL,4			; YES
STOP1:	MOV	DX,LCR[SI]		; LINE CONTROL REGISTER
	OUT	DX,AL			; SET UART PARITY MODE AND DLAB=0

; Initialize the FIFOs
	
	CMP UART_SILO_LEN[SI],1		; Is it a 16550A?
	 JE P5				; jump if 8250, 16450, or 16550(no A)
	MOV	DX,FCR[SI]		; I/O Address of FIFO control register
	MOV	AL,FIFO_INIT		; Clear FIFOs, set size, enable FIFOs
	OUT	DX,AL
	JMP SHORT $+2

; ENABLE INTERRUPTS ON 8259 AND UART
P5:	IN	AL,INTA01		; SET ENABLE BIT ON 8259
	AND	AL,NIRQ[SI]
	JMP SHORT $+2
	OUT	INTA01,AL
	MOV DX,IER[SI]			; Interrupt enable register
	MOV AL,0DH			; Line & Modem status, recv [GT]
	OUT DX,AL			; Enable those interrupts
	MOV IER_SHADOW[SI],AL

OPENX:	POPF				; Restore interrupt state
	POP SI
	mov sp,bp
	pop bp
	RET				; DONE
_open_com ENDP
	PAGE;
;
; void far ioctl_com(int Flags, int Arg1, ...)
;	Flags have bits saying what to do or change (IGNORED TODAY)
;	Arg1, ...  are the new values
;
_ioctl_com PROC FAR
	PUSH BP
	MOV BP,SP
	PUSH SI
	MOV SI,CURRENT_AREA		; Pointer to COMi private area
	TEST INSTALLED[SI],1
	 JE IOCTLX			; No good.  Just return.
	PUSHF				; Save interrupt context
	CLI				; Prevent surprises
	; MOV AX,[BP+6]			; Flags
	; Check bits here...
	MOV AX,[BP+8]			; Line speed
	MOV BAUD_RATE[SI],AX		; Save in parameter block
	CALL Set_Baud			; Set the baud rate in UART
	POPF				; Restore interrupt state
IOCTLX:	POP SI
	MOV SP,BP
	POP BP
	RET
_ioctl_com	ENDP
	PAGE;
; ioctl-called routines (internal) ...

; SI:	COMi private block
;	CALL Set_Baud
; Returns: (nothing)
; Clobber: AX, BX, DX

Set_Baud PROC NEAR
	MOV AX,50
	MUL DIV50			; Could be different on a PCJr!
	DIV BAUD_RATE[SI]		; Get right number for the UART
	MOV BX,AX			; Save it
	MOV DX,LCR[SI]			; Line Control Register
	IN AL,DX			; Get current size, stops, parity,...
	MOV AH,AL
	OR AL,80H			; DLAB bit
	JMP SHORT $+2
	OUT DX,AL			; Talk to the baud rate regs now
	MOV DX,WORD PTR DLL[SI] 	; Least significant byte
	MOV AL,BL			; New value
	JMP SHORT $+2
	OUT DX,AL			; To UART
	MOV DX,WORD PTR DLH[SI] 	; Most significant byte
	MOV AL,BH			; New value
	JMP SHORT $+2
	OUT DX,AL
	MOV DX,LCR[SI]			; Line Control Register
	MOV AL,AH
	JMP SHORT $+2
	OUT DX,AL			; Turn off DLAB, keep saved settings
	RET
Set_Baud ENDP
	PAGE;
;
; void far close_com(void)
;	Turn off interrupts from the COM port
;
_close_com PROC FAR
	PUSH SI
	PUSHF
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	CCX			; ABORT IF NOT

; TURN OFF UART and clear FIFOs in NS16550A
	CLI
	MOV DX,IER[SI]
	XOR AL,AL
	OUT DX,AL			; No interrupts right now, please
	MOV IER_SHADOW[SI],AL
	MOV DX,FCR[SI]			; FIFO Control Register
	MOV AL,FIFO_CLEAR		; Disable FIFOs
	JMP SHORT $+2
	OUT DX,AL
	MOV DX,MCR[SI]			; Modem control register
	XOR AL,AL			; OUT2 is IRQ enable on some machines,
	JMP SHORT $+2
	OUT DX,AL			; So, clear RTS, OUT1, OUT2, LOOPBACK

; TURN OFF 8259
	MOV	DX,INTA01
	JMP SHORT $+2
	IN	AL,DX
	OR	AL,IRQ[SI]
	JMP SHORT $+2
	OUT	DX,AL

CCX:	POPF				; Restore interrupt state
	POP SI
	RET
_close_com ENDP
	PAGE;
;
; void far dtr_off(void)
;	Tells modem we are done.  Remote end should hang up also.
;
_dtr_off PROC FAR
	PUSH SI
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	DFX			; ABORT IF NOT

	MOV DX,MCR[SI]			; Modem Control Register
IFNDEF UUPC
	IN AL,DX			; Get current state
	SHR AL,1			; Save old DTR bit in Carry flag
	JMP SHORT $+2
ENDIF
	MOV AL,08H			; DTR off, RTS off, OUT2 on
	OUT DX,AL
IFNDEF UUPC
	 JNC DFX			; if DTR wasn't on then don't wait.
	MOV AX,50			; 50/100 of second
	CALL WaitN			; V.24 says it must be low >1/2 sec
ENDIF
DFX:	POP SI
	RET
_dtr_off	ENDP
	PAGE;
;
; void far dtr_on(void) 	Tell modem we can take traffic
;
_dtr_on PROC FAR
	PUSH SI
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ DTRONF			; Suppress output if not

; Tell modem we are ready and want to send with line idle

	MOV DX,MCR[SI]			; Modem Control Register
	MOV AL,00001011B		; OUT 2, RTS, DTR
	OUT DX,AL
	CMP CONNECTION[SI],'D'          ; Direct connection (no DSR,CTS)?
	 JE DTRDIR			; if so, then don't wait

; Wait for awhile to give the modem time to respond

	MOV AH,2CH			; Get time (H:M:S:H to CH:CL:DH:DL)
	INT 21H
	MOV BX,DX			; Save seconds&hundreths
	ADD BH,06			; Allow a few seconds
	CMP BH,60			; Wrap around check
	 JL DTRON1			; No wrap
	SUB BH,60
DTRON1: CMP SEND_OK[SI],30H		; Did the modem come up?
	 JE DTRONS			; Yes.	Both DSR and CTS are true.
	INT 21H 			; Get the time again
	CMP DX,BX			; Current time is passed the deadline?
	 JB DTRON1			; No, keep checking 'til time runs out

	; Modem failed to come up.  Bump counts that tell why.
	MOV	DX,MSR[SI]		; MODEM STATUS REGISTER
	IN	AL,DX			; GET MODEM STATUS
	TEST	AL,20H			; DATA SET READY?
	 JNZ DTRON6			; Yup.
	INC	WORD PTR EDSR[SI]	; BUMP ERROR COUNT
DTRON6: TEST	AL,10H			; Clear To Send?
	 JNZ DTRONF			; That's OK.
	INC	WORD PTR ECTS[SI]	; BUMP ERROR COUNT - WE TIMED OUT
	; Fall into DTRONF
	PAGE;
; Failure return

DTRONF:	MOV CONNECTION[SI],'D'          ; Switch to DIR connection (MSTATINT)
	; Fall into DTRDIR

DTRDIR:	MOV SEND_OK[SI],30H		; Make believe DSR & CTS are up!!!
	; Fall into DTRONS

; Successful return

DTRONS: ; SEND_OK is on.  Setting it again could confuse interrupt level
	; Fall into DTRONX

DTRONX:
IFNDEF UUPC
	MOV AX,200H			; 2 Seconds
	CALL WaitN			; V.24 says 2 sec hi before data
ENDIF
	POP SI
	RET
_dtr_on ENDP
	PAGE;
;
; void far rts_off(void)
;	Turns off RTS for RTS-style throttling of modem->PC data
;
_rts_off PROC FAR
	PUSH SI
	MOV	SI,CURRENT_AREA		; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1		; PORT INSTALLED?
	 JZ	RFX			; ABORT IF NOT
	MOV DX,MCR[SI]			; Modem Control Register
	MOV AL,00001001B		; OUT 2, DTR
	OUT DX,AL
RFX:	POP SI
	RET
_rts_off	ENDP
	PAGE;
;
; void far rts_off(void)
;	Turns on RTS for RTS-style throttling of modem->PC data
;
_rts_on PROC FAR
	PUSH SI
	MOV	SI,CURRENT_AREA		; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1		; PORT INSTALLED?
	 JZ	RNX			; ABORT IF NOT
	MOV DX,MCR[SI]			; Modem Control Register
	MOV AL,00001011B		; OUT 2, RTS, DTR
	OUT DX,AL
RNX:	POP SI
	RET
_rts_on	ENDP
	PAGE;
;
; Wait for specified time using the 18.2 ticks/second clock
;
; Call: 	AX has seconds,hundreths
;		CALL WaitN
; Return:	At least the requested time has passed
;

WaitN	PROC NEAR
	PUSH AX
	PUSH BX
	PUSH CX
	PUSH DX
	PUSH AX 			; Save a copy of the arg
	MOV AH,2CH			; Get time (H:M:S:H to CH:CL:DH:DL)
	INT DOS
	POP BX				; Recover S:H arg
	ADD BX,DX			; Determine deadline
	CMP BL,100			; Wrap around?
	 JL WaitN1			; No
	SUB BL,100			; Yes.	Subtract 100 hundreths
	INC BH				; And add a second
WaitN1: CMP BH,60			; Wrap around check
	 JL WaitN2			; No wrap
	SUB BH,60			; Forget about Days and Hours
WaitN2: INT DOS 			; Get the time again
	CMP DX,BX			; Is current time after the deadline?
	 JB WaitN2			; No, keep checking 'til time runs out
	POP DX
	POP CX
	POP BX
	POP CX
	RET
WaitN	ENDP
	PAGE;
;
; unsigned long r_count(void)
;	Value is really two uints:  Buffer size in high half, count in low.
;	Count returned is <= number of chars waiting to be read.
;		(More may come in after you asked.)
;
_r_count PROC FAR
	PUSH SI
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	XOR	AX,AX			; Say nothing available if not inst'd
	MOV DX,R_SIZE			; Size of entire receive buffer
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	RCX			; ABORT IF NOT
	MOV	AX,SIZE_RDATA[SI]	; GET NUMBER OF BYTES USED
RCX:	POP SI
	RET
_r_count ENDP
	PAGE;
;
; char far receive_com(void)
;	Returns AX: -1 if port not installed or no characters available
;		or AX: the next character with parity stipped if not in P mode
;
_receive_com PROC FAR
	PUSH SI
	PUSH ES
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	mov	ax,-1			; -1 if bad call
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	RECVX			; ABORT IF NOT
	CMP	SIZE_RDATA[SI],0	; ANY CHARACTERS?
	 JE	RECVX			; Return -1 in AX

	mov ah,0			; good call
	LES	BX,START_RDATA[SI]	; GET POINTER TO OLDEST CHAR
	MOV	AL,ES:[BX]		; Get character from buffer
	INC	BX			; BUMP START_RDATA
	AND	BX,R_SIZE-1		; Ring the pointer
	MOV	WORD PTR START_RDATA[SI],BX ; SAVE THE NEW START_RDATA VALUE
	DEC	SIZE_RDATA[SI]		; ONE LESS CHARACTER
	CMP	PARITY[SI],'N'          ; ARE WE RUNNING WITH NO PARITY? LBA
	 JE	RECV1			; IF SO, DON'T STRIP HIGH BIT    LBA
	AND	AL,7FH			; STRIP PARITY BIT
RECV1:	CMP	XON_XOFF[SI],'E'        ; XON/XOFF FLOW CONTROL ENABLED?
	 JNE	RECVX			; DO NOTHING IF DISABLED
	CMP	HOST_OFF[SI],1		; HOST TURNED OFF?
	 JNE	RECVX			; JUMP IF NOT
	CMP	SIZE_RDATA[SI],R_SIZE/16; RECEIVE BUFFER NEARLY EMPTY?
	 JAE	RECVX			; DONE IF NOT
	MOV	HOST_OFF[SI],0		; TURN ON HOST IF SO

	PUSH	AX			; SAVE RECEIVED CHAR
	MOV	AL,CONTROL_Q		; TELL HIM TO TALK
	PUSHF				; Save interrupt context
RECV3:	CLI				; TURN OFF INTERRUPTS
	CMP URGENT_SEND[SI],1		; Previous send still in progress?
	 JNE RECV4			; No.  There is space now.
	STI				; Yes.	Wait for interrupt to take it.
	JMP SHORT $+2			; Waste some time with interrupts on
	JMP SHORT RECV3 		; Loop 'til it's gone

RECV4:	CALL	SENDII			; SEND IMMEDIATELY INTERNAL
	POPF				; Restore interrupt state
	POP	AX			; RESTORE RECEIVED CHAR

RECVX:	POP ES
	POP SI
	RET
_receive_com ENDP
	PAGE;
;
; unsigned long s_count(void)
;    Value is really two uints: Buffer size in high half (returned in DX).
;				Free space count in low (returned in AX).
;    Count returned is <= number of chars which can be sent without blocking.
;		(More may become available after you asked.)
;
; N.B. The free space might be negative (-1) if the buffer was full and then
; the program called SENDI or RXI required sending a control-S to squelch
; the remote sender.  Return 0 in this case.
;
_s_count PROC FAR
	PUSH SI
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	XOR	AX,AX			; NO SPACE LEFT IF NOT INSTALLED
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	SCX			; ABORT IF NOT
	MOV AX,S_SIZE-1 		; Size, keeping one aside for SENDII
	SUB AX,SIZE_TDATA[SI]		; Minus number in use right now
	CWD				; Avoid returning a negative number
	NOT DX
	AND AX,DX			; return 0 if it was negative
SCX:	MOV DX,S_SIZE-1			; Leave 1 byte for a SENDII call
	POP SI
	RET
_s_count ENDP
	PAGE;
;
; void far send_com(char)
;	Send a character to the selected port
;
_send_com PROC FAR
	push bp
	mov bp,sp
	PUSH SI
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	SENDX			; ABORT IF NOT

SEND1:	CMP	SIZE_TDATA[SI],S_SIZE-1 ; BUFFER FULL? (Leave room for SENDII)
	 JAE SEND1			; Wait for interrupts to empty buffer
	MOV AL,[BP+6]			; Character to send
	PUSH ES
	LES BX,END_TDATA[SI]		; ES:BX points to free space
	MOV ES:[BX],AL			; Move character to buffer
	INC	BX			; INCREMENT END_TDATA
	AND BX,S_SIZE-1 		; Ring the pointer
	MOV WORD PTR END_TDATA[SI],BX	; SAVE NEW END_TDATA
	INC	SIZE_TDATA[SI]		; ONE MORE CHARACTER IN X-MIT BUFFER
	TEST PC_OFF[SI],1		; Were we stopped by a ^S from host?
	 JNZ SEND2			; Yes.	Don't enable interrupts yet.
	CMP SEND_OK[SI],30H		; See if Data Set Ready & CTS are on
	 JNE SEND2			; No. Still can't enable TX ints
	PUSHF				; Save interrupt state
	CLI
	TEST IER_SHADOW[SI],02H		; Tx interrupts enabled?
	 JNZ SEND3			; Jump if so: transmit already running
	MOV DX,IER[SI]			; Interrupt Enable Register
	MOV AL,0FH			; Rx, Tx, Line & Modem enable bits
	OUT DX,AL			; Enable those interrupts
	MOV IER_SHADOW[SI],AL
SEND3:	POPF				; Restore interrupt state
SEND2:	POP ES
SENDX:	POP SI
	mov sp,bp
	pop bp
	RET
_send_com ENDP
	PAGE;
;
; void far sendi_com(char)
;	Send a character immediately by placing it at the head of the queue
;
_sendi_com PROC FAR
	push bp
	mov bp,sp
	PUSH SI
	mov al,[bp+6]
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ SENDIX			; Return if not
	PUSHF				; Save interrupt state
SENDI3: CLI				; TURN OFF INTERRUPTS
	CMP URGENT_SEND[SI],1		; Previous send still in progress?
	 JNE SENDI4			; No.  There is space now.
	STI				; Yes.	Wait for interrupt to take it.
	JMP SHORT $+2			; Waste some time with interrupts on
	JMP SHORT SENDI3		; Loop 'til it's gone

SENDI4: CALL	SENDII			; CALL INTERNAL SEND IMMEDIATE
	POPF				; Restore interrupt state
SENDIX:	POP SI
	mov sp,bp
	pop bp
	RET
_sendi_com ENDP
	PAGE;
; SENDII(AL, SI)  [internal routine]
;	Put char at head of output queue so it will go out next
;	Called from process level and (receive) interrupt level
;	DEPENDS ON CALLER TO KEEP INTERRUPTS CLEARED AND SET SI
;	Clobbers AL
;
SENDII	PROC NEAR
	TEST	IER_SHADOW[SI],02H	; Tx interrupts enabled?
	 JNZ	SENDII1			; Jump if so: transmit already running
	CMP	SEND_OK[SI],30H		; See if Data Set Ready & CTS are on
	 JNE	SENDII1			; No. Still can't enable TX ints
; Was idle: transmit the character without buffering; enable TX interrupt
	PUSH	DX
	MOV	DX,DATREG[SI]		; I/O address of data register
	OUT	DX,AL			; Output the character
	MOV	DX,IER[SI]		; Interrupt Enable Register
	MOV	AL,0FH			; Rx, Tx, Line & Modem enable bits
	OUT	DX,AL			; Enable those interrupts
	MOV	IER_SHADOW[SI],AL
	POP	DX
	RET

; Unable to send the character now: buffer it.
SENDII1:PUSH BX
	PUSH ES
	LES BX,START_TDATA[SI]		; ES:BX point to 1st chr in buffer
	CMP	SIZE_TDATA[SI],S_SIZE	; BUFFER FULL?
	 JB	SENDII2 		; JUMP IF NOT
	INC	WORD PTR EOVFLOW[SI]	; BUMP ERROR COUNT (Can this happen?)
	JMP SHORT SENDII4		; and overwrite old first char

SENDII2:DEC BX				; Back it up
	AND BX,S_SIZE-1 		; Ring it
	MOV WORD PTR START_TDATA[SI],BX	; Save new value
	INC	SIZE_TDATA[SI]		; ONE MORE CHARACTER IN X-MIT BUFFER
	; No check for PC_OFF here.  Flow control ALWAYS gets sent!
SENDII4:MOV ES:[BX],AL			; Move character to buffer
	MOV URGENT_SEND[SI],1		; Flag high priority message
	POP ES
	POP BX
	RET
SENDII	ENDP
	PAGE;
;*---------------------------------------------------------------------*
;*	s p i n l o o p 					       *
;*								       *
;*	Waste time to allow slow UART chips to catch up 	       *
;*---------------------------------------------------------------------*

SPINLOOP PROC NEAR
	PUSH CX
	MOV CX,100
WASTERS:
	CALL CRET		; Better time waster than a 1-byte NOP
	LOOP WASTERS
	POP  CX
CRET:	RET
SPINLOOP ENDP
	PAGE;

IFNDEF UUPC
;
; void far send_local(char);
;	Simulate a loopback by placing characters sent in recv buffer
;
_send_local PROC FAR
	push bp
	mov bp,sp
	PUSH SI
	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	SLX			; ABORT IF NOT

	PUSHF				; Save interrupt state
	CLI				; INTERRUPTS OFF

	CMP	SIZE_RDATA[SI],R_SIZE	; SEE IF ANY ROOM
	 JB SL3 			; SKIP IF ROOM
	INC	WORD PTR EOVFLOW[SI]	; BUMP OVERFLOW COUNT
	JMP SHORT SL9			; PUNT

SL3:	PUSH ES
	LES BX,END_RDATA[SI]		; ES:BX POINTS TO FREE SPACE
	MOV AL,[BP+6]			; Get the byte to send
	MOV ES:[BX],AL			; Put into buffer
	INC	BX			; INCREMENT END_RDATA POINTER
	AND BX,R_SIZE-1 		; Ring the pointer
	MOV WORD PTR END_RDATA[SI],BX	; SAVE VALUE
	INC	SIZE_RDATA[SI]		; GOT ONE MORE CHARACTER
	POP ES

SL9:	POPF				; Restore interrupt state
SLX:	POP SI
	mov sp,bp
	pop bp
	RET				; DONE
_send_local	ENDP
ENDIF
	PAGE;
;
; void far break_com(void)	Send a BREAK out to alert the remote end
;
_break_com PROC FAR
	PUSH SI

	MOV	SI,CURRENT_AREA 	; SI POINTS TO DATA AREA
	TEST	INSTALLED[SI],1 	; PORT INSTALLED?
	 JZ	BREAKX			; ABORT IF NOT

	MOV	DX,LCR[SI]		; LINE CONTROL REGISTER
	IN	AL,DX			; GET CURRENT SETTING
	OR	AL,40H			; TURN ON BREAK BIT
	JMP SHORT $+2
	OUT	DX,AL			; SET IT ON THE UART
	MOV AX,25			; 25/100 of a second
	CALL WaitN
	MOV	DX,LCR[SI]		; LINE CONTROL REGISTER
	IN	AL,DX			; GET CURRENT SETTING
	AND	AL,NOT 40H 		; TURN OFF BREAK BIT
	JMP SHORT $+2
	OUT	DX,AL			; RESTORE LINE CONTROL REGISTER
BREAKX: POP SI
	RET
_break_com ENDP
	PAGE;
;
; ERROR_STRUCT far *com_errors(void)
;	Returns a pointer to the table of error counters
;
_com_errors PROC FAR
	PUSH SI
	MOV SI,CURRENT_AREA		; Point to block for selected port
	LEA AX,ERROR_BLOCK[SI]		; Offset to error counters
	MOV DX,DS			; Value is in DX:AX
	POP SI
	RET
_com_errors ENDP






;
; char far modem_status(void)
;	Returns the modem status register in AL
;
; Bits are:	0x80:	Carrier Detect
;		0x40:	Ring Indicator
;		0x20:	Data Set Ready
;		0x10:	Clear To Send
;		0x08:	Delta Carrier Detect	(CD changed)
;		0x04:	Trailing edge of RI	(RI went OFF)
;		0x02:	Delta DSR		(DSR changed)
;		0x01:	Delta CTS		(CTS changed)

_modem_status PROC FAR
	PUSH SI
	MOV SI,CURRENT_AREA		; Point to block for selected port
	MOV DX,MSR[SI]			; IO Addr of Modem Status Register
	IN AL,DX			; Get the live value
	XOR AH,AH			; Flush unwanted bits
	POP SI
	RET
_modem_status ENDP
	PAGE;
;
; INT_HNDLR1 - HANDLES INTERRUPTS GENERATED BY COM1:
;
	assume	DS:nothing
	even
INT_HNDLR1 PROC FAR
	PUSH SI
	MOV	SI,OFFSET DGROUP:AREA1	; DATA AREA FOR COM1:
	JMP	SHORT INT_COMMON	; CONTINUE
;
; INT_HNDLR2 - HANDLES INTERRUPTS GENERATED BY COM2:
;
	even
INT_HNDLR2 PROC FAR
	PUSH SI
	MOV	SI,OFFSET DGROUP:AREA2	; DATA AREA FOR COM2:
	JMP	SHORT INT_COMMON	; CONTINUE
;
; INT_HNDLR3 - HANDLES INTERRUPTS GENERATED BY COM3:
;
	even
INT_HNDLR3 PROC FAR
	PUSH SI
	MOV	SI,OFFSET DGROUP:AREA3	; DATA AREA FOR COM3:
	JMP	SHORT INT_COMMON	; CONTINUE
;
; INT_HNDLR4 - HANDLES INTERRUPTS GENERATED BY COM4:
;
	even
INT_HNDLR4 PROC FAR
	PUSH SI
	MOV	SI,OFFSET DGROUP:AREA4	; DATA AREA FOR COM4:
	; Fall into INT_COMMON
	PAGE;
;
; BODY OF INTERRUPT HANDLER
;
INT_COMMON: ; SI has been pushed and loaded
	PUSH AX
	PUSH DX
	PUSH DS
	PUSH BX
	PUSH CX
	PUSH ES

	MOV AX,DGROUP			; offsets are relative to DGROUP [WWP]
	MOV	DS,AX
	assume	DS:DGROUP

; FIND OUT WHERE INTERRUPT CAME FROM AND JUMP TO ROUTINE TO HANDLE IT
REPOLL: MOV DX,IIR[SI]			; Interrupt Identification Register
	IN AL,DX
	MOV BL,AL			; Put where we can index by it
	SHR AL,1			; Check the "no interrupt present" bit
	 JC INT_END			; ON means we are done
	AND BX,000EH			; Ignore FIFO_ENABLED bits, etc.

	JMP WORD PTR CS:INT_DISPATCH[BX]; Go to appropriate routine

	even
INT_DISPATCH:
	DW MSI				; 0: Modem status interrupt
	DW TXI				; 2: Transmitter interrupt
	DW RXI				; 4: Receiver interrupt
	DW LSI				; 6: Line status interrupt
	DW INT_END			; 8: (Future use by UART makers)
	DW INT_END			; A: (Future use by UART makers)
	DW RXI				; C: FIFO Timeout
	DW INT_END			; E: (Future use by UART makers)

INT_END: ; Now tell 8259 we handled that IRQ

	MOV DX,IER[SI]			; Gordon Lee's cure for dropped ints
	XOR AL,AL
	OUT DX,AL			; Disable UART interrupts
	MOV AL,EOI[SI]			; End of Interrupt. With input gone,
	POP ES				; do the POPs now for some I/O delay
	POP CX
	OUT INTA00,AL			; Give EOI to 8259 Interrupt ctlr
	MOV AL,IER_SHADOW[SI]		; Get saved interrupt enable bits
	POP BX				; do the POPs now for some I/O delay
	POP DS
	assume	DS:nothing
	OUT DX,AL			; Restore them, maybe causing a
					; transition into the 8259!!!
	POP DX
	POP AX
	POP SI
	IRET
	PAGE;
;
; Line status interrupt
;
	even
	assume	DS:DGROUP
LSI:	MOV	DX,LSR[SI]		; Line status register
	IN	AL,DX			; Read line status & bump error counts
	SHR	AL,1
LSI2:	; RXI also comes here with the LSR (shifted right 1) already in AL
	SHR	AL,1			; OVERRUN ERROR?
	ADC	WORD PTR EOVRUN[SI],0	; BUMP ERROR COUNT
	SHR	AL,1			; PARITY ERROR?
	ADC	WORD PTR EPARITY[SI],0	; BUMP ERROR COUNT
	SHR	AL,1			; FRAMING ERROR?
	ADC	WORD PTR EFRAME[SI],0	; BUMP ERROR COUNT
	SHR	AL,1			; BREAK RECEIVED?
	ADC	WORD PTR EBREAK[SI],0	; BUMP ERROR COUNT
	JMP	REPOLL			; SEE IF ANY MORE INTERRUPTS

;
; Modem status interrupt
;
	even
	assume	DS:DGROUP
MSI:	MOV DX,MSR[SI]			; Modem Status Register
	IN AL,DX			; Read status & clear interrupt
	AND AL,30H			; Expose CTS and Data Set Ready
	CMP CONNECTION[SI],'D'          ; Direct connection - ignore int
	 JNE MSI0			; if not direct them DSR,CTS valid
	MOV AL,30H			; Make believe DSR & CTS are up!!!
MSI0:	MOV SEND_OK[SI],AL		; Put where TXI and send_com can see
	MOV DX,IER[SI]			; Let a TX int happen for thoro chks
	MOV AL,0FH			; Line & modem sts, recv, send.
	OUT DX,AL
	MOV IER_SHADOW[SI],AL
	JMP REPOLL			; Check for other interrupts
	PAGE;
;
; Receive interrupt
;
	even
	assume	DS:DGROUP
RXI:	LES	BX,END_RDATA[SI]	; ES:BX points to free space
RXI0:	MOV	DX,DATREG[SI]		; UART DATA REGISTER
	IN	AL,DX			; Get data, clear status
	CMP	XON_XOFF[SI],'E'        ; XON/XOFF FLOW CONTROL ENABLED?
	 JE	RXI1			; Yes.  Check for XON/XOFF
	CMP	SIZE_RDATA[SI],R_SIZE	; Any room in buffer?
	 JAE	RXIFUL			; No room left
	MOV	ES:[BX],AL		; Put character in buffer
	INC	SIZE_RDATA[SI]		; GOT ONE MORE CHARACTER
	INC	BX			; Bump pointer past location just used
	AND	BX,R_SIZE-1		; Ring the pointer
RXINXT:	MOV	DX,LSR[SI]		; Line Status Register
	IN	AL,DX			; Read it
	SHR	AL,1			; Check the RECV DATA READY bit
	 JC	RXI0			; More data; go get it
	MOV	WORD PTR END_RDATA[SI],BX ; Store updated pointer
	JMP	SHORT LSI2		; No more data; count errors if any

RXIFUL:	INC	WORD PTR EOVFLOW[SI]	; BUMP OVERFLOW ERROR COUNT
	MOV	WORD PTR END_RDATA[SI],BX ; Store updated pointer
	JMP	REPOLL

; Check each character for possible flow control (XON/XOFF)
RXI1:	MOV	AH,AL			; Save character in AH
	AND	AL,7FH			; STRIP PARITY
	CMP	AL,CONTROL_S		; STOP COMMAND RECEIVED?
	 JNE	RXI2			; Jump if not. Might be ^Q though.
	MOV	PC_OFF[SI],1		; Stop output
	JMP	SHORT RXINXT		; Don't store the character

RXI2:	CMP	AL,CONTROL_Q		; GO COMMAND RECEIVED?
	 JNE	RXI3			; No.  Not a flow control character
	MOV	PC_OFF[SI],0		; Enable output
	JMP	SHORT RXINXT		; Don't store the character

RXI3:	CMP	SIZE_RDATA[SI],R_SIZE	; SEE IF ANY ROOM
	 JAE	RXIFUL			; No room left
	MOV	ES:[BX],AH		; Put saved character in buffer
	INC	SIZE_RDATA[SI]		; GOT ONE MORE CHARACTER
	INC	BX			; Bump pointer past location just used
	AND	BX,R_SIZE-1		; Ring the pointer
; See if we must tell remote host to stop outputting.
	CMP	HOST_OFF[SI],1		; Already told remote to shut up?
	 JE	RXINXT			; Yes.	Don't flood him with ^Ss
	CMP	SIZE_RDATA[SI],R_SIZE/2 ; RECEIVE BUFFER NEARLY FULL?
	 JBE	RXINXT			; No.  No need to stifle remote end
	; Would like to wait here for URGENT_SEND to go off if it is on.
	; But we need to take a TX interrupt for that to happen.
	MOV	AL,CONTROL_S		; TURN OFF HOST IF SO
	CALL	SENDII			; SEND IMMEDIATELY INTERNAL
	MOV	HOST_OFF[SI],1		; HOST IS NOW OFF
	JMP	RXINXT
	PAGE;
;
; Transmit interrupt
;
	even
	assume	DS:DGROUP
TXI:	CMP	SEND_OK[SI],30H		; Hardware (CTS & DSR on) OK?
	 JNE	TXI9			; No.  Must wait 'til cable right!
	MOV	CX,1			; Transfer count for flow ctl
	CMP	URGENT_SEND[SI],CL	; Flow control character to send?
	 JE	TXI3			; Yes.	Always send flow control.
	CMP	PC_OFF[SI],CL		; Flow control (XON/XOFF) OK?
	 JE	TXI9			; Stifled & not urgent. Forget it.

TXI1:	MOV	CL,UART_SILO_LEN[SI]	; MAX size chunk (1 for 8250/16450)
	; Too bad there is no "Transmitter FIFO Full" indication!
	CMP	SIZE_TDATA[SI],CX	; SEE IF ANY MORE DATA TO SEND
	 JAE	TXI3			; jump if UART is the limit
	MOV	CX,SIZE_TDATA[SI]	; Buffer contents limited.  Use that.
	 JCXZ	TXI9			; No data, disable TX ints
TXI3:	LES	BX,START_TDATA[SI]	; ES:BX points to next char to send
	MOV	DX,DATREG[SI]		; I/O address of data register
TXI4:	MOV	AL,ES:[BX]		; Get character from buffer
	OUT	DX,AL			; Output the character
	INC	BX			; Bump the head pointer
	AND	BX,S_SIZE-1		; Ring it
	DEC	SIZE_TDATA[SI]		; One fewer in buffer now
	 LOOP	TXI4			; Keep going 'til silo is full
	MOV	WORD PTR START_TDATA[SI],BX ; Store pointer back
	MOV	URGENT_SEND[SI],0	; Tell process level we sent flow ctl
	JMP	REPOLL

; IF NO DATA TO SEND, or can't send, RESET TX INTERRUPT AND RETURN
TXI9:	MOV	DX,IER[SI]
	MOV	AL,0DH			; Line & modem sts, recv, no send.
	OUT	DX,AL
	MOV	IER_SHADOW[SI],AL
	JMP	REPOLL

INT_HNDLR4 ENDP
INT_HNDLR3 ENDP
INT_HNDLR2 ENDP
INT_HNDLR1 ENDP
COM_TEXT   ENDS
	   END
