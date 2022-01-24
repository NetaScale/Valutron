#include <iostream>

#include "Interpreter.hh"
#include "ObjectMemory.hh"

void
disassemble(uint8_t *code, int length) noexcept
{
	int pc = 0;

	std::cout << "Disassembly:\n";

#define FETCH code[pc++]
	while (pc < length) {
		Op::Opcode op = (Op::Opcode)FETCH;

		std::cout << pc - 1 << "\t";

		switch (op) {
		/* u8 index/reg, u8 dest */
		case Op::kMoveParentHeapVarToMyHeapVars: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "HeapVars at: " << dst <<
			    " put: (parentHeapVars at:" << dst << ").\n";
			break;
		}

		case Op::kMoveMyHeapVarToParentHeapVars: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "parentHeapVars at: " << dst <<
			    " put: (heapVars at: " << src << ").\n";
			break;
		}

		case Op::kLdaNil:
			std::cout << "ac <- nil.\n";
			break;

		case Op::kLdaTrue:
			std::cout << "ac <- true.\n";
			break;

		case Op::kLdaFalse:
			std::cout << "ac <- false.\n";
			break;

		case Op::kLdaThisContext:
			std::cout << "ac <- thisContext.\n";
			break;

		case Op::kLdaThisProcess:
			std::cout << "ac <- thisProcess.\n";
			break;

		case Op::kLdaSmalltalk:
			std::cout << "ac <- smalltalk.\n";
			break;

		/* u8 index*/
		case Op::kLdaParentHeapVar: {
			unsigned src = FETCH;
			std::cout << "ac <- parentHeapVars at: " << src << ".\n";
			break;
		}

		case Op::kLdaMyHeapVar: {
			unsigned src = FETCH;
			std::cout << "ac <- heapVars at: " << src << ".\n";
			break;
		}

		case Op::kLdaGlobal: {
			unsigned src = FETCH;
			std::cout << "ac <- ObjectMemory lookupGlobal: lit" <<
			    src << ".\n";
			break;
		}

		case Op::kLdaNstVar: {
			unsigned src = FETCH;
			std::cout << "ac <- receiver instVarAt: " << src <<
			    ".\n";
			break;
		}

		case Op::kLdaLiteral: {
			unsigned src = FETCH;
			std::cout << "ac <- lit" << src << ".\n";
			break;
		}

		case Op::kLdaBlockCopy: {
			unsigned src = FETCH;
			std::cout << "ac <- lit" << src << " blockCopy.\n";
			break;
		}

		case Op::kLdar: {
			unsigned src = FETCH;
			std::cout << "ac <- r" << src << "\n";
			break;
		}

		/* u8 index */
		case Op::kStaNstVar: {
			unsigned dst = FETCH;
			std::cout << "receiver instVarAt: " << dst <<
			    " put: ac.\n";
			break;
		}

		case Op::kStaGlobal: {
			unsigned dst = FETCH;
			std::cout << "ObjectMemory at: l" << dst <<
			    " put: ac.\n";
			break;
		}

		case Op::kStaParentHeapVar: {
			unsigned dst = FETCH;
			std::cout << "parentHeapVars at: " << dst <<
			    " put: ac.\n";
			break;
		}

		case Op::kStaMyHeapVar: {
			unsigned dst = FETCH;
			std::cout << "heapVars at: " << dst << " put: ac.\n";
			break;
		}

		case Op::kStar: {
			unsigned dst = FETCH;
			std::cout << "r" << dst << " <- ac.\n";
			break;
		}

		case Op::kMove: {
			unsigned dst = FETCH, src = FETCH;
			std::cout << "r" << dst <<
			    " <- r" << src << "\n";
			break;
		}

		/* a value, u8 src-reg */
		case Op::kAnd: {
			unsigned src = FETCH;
			std::cout << "ac <- ac and: r" << src <<".\n";
			break;
		}

		/* i16 pc-offset */
		case Op::kJump: {
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			std::cout << "self jump: " << pc + offs << ".\n";
			break;
		}

		/* a value, i16 pc-offset */
		case Op::kBranchIfFalse: {
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			std::cout << "ac ifFalse: [ac branch: " <<
			    pc + offs << "].\n";
			break;
		}

		/* a value, i16 pc-offset */
		case Op::kBranchIfTrue: {
			uint8_t b1 = FETCH;
			uint8_t b2 = FETCH;
			int16_t offs = (b1 << 8) | b2;
			std::cout << "ac ifTrue: [self branch: " <<
			    pc + offs << "].\n";
			break;
		}

		case Op::kBinOp: {
			unsigned src = FETCH;
			unsigned op = FETCH;
			std::cout << "self binOp: #" <<
			    ObjectMemory::binOpStr[op] << " on: ac arg: "
			    " r" << src << "\n";
			break;
		}

		/**
		 * u8 dest-reg, u8 receiver-reg, u8 selector-lit-idx,
		 * u8 num-args, (u8 arg-reg)+
		 */
		case Op::kSend: {
			unsigned selIdx = FETCH, nArgs = FETCH;
			std::cout << "self sendTo: ac cache: l" << selIdx;

			if (nArgs > 0) {
				std::cout << " args: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << unsigned(FETCH);
				}
				std::cout << "]";
			}

			std::cout << ".\n";

			break;
		}

		/**
		 * u8 dest-reg, u8 selector-literal-index, u8 num-args,
		 * (u8 arg-register)+
		 */
		case Op::kSendSuper: {
			unsigned selIdx = FETCH, nArgs = FETCH;
			std::cout << "self sendSuperTo: ac cache: l" << selIdx;

			if (nArgs > 0) {
				std::cout << " args: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << FETCH;
				}
				std::cout << "]";
			}

			std::cout << ".\n";
			break;
		}

		/** u8 prim-num, u8 num-args, (u8 arg-reg)+ */
		case Op::kPrimitive: {
			unsigned prim = FETCH, nArgs = FETCH;
			std::cout << "self primitive" << prim << "";

			if (nArgs > 0) {
				std::cout << "With: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << (unsigned)FETCH;
				}
				std::cout << "]";
			}

			std::cout << ".\n";
			break;
		}

		/** a arg, u8 prim-num */
		case Op::kPrimitive0: {
			unsigned prim = FETCH;
			std::cout << "self primitive" << prim << ".\n";
			break;
		}

		/** a arg, u8 prim-num */
		case Op::kPrimitive1: {
			unsigned prim = FETCH;
			std::cout << "self primitive" << prim <<
			    "with: ac.\n";
			break;
		}

		/** a arg2, u8 prim-num, u8 arg1-reg, */
		case Op::kPrimitive2: {
			unsigned prim = FETCH, arg2 = FETCH;
			std::cout << "self primitive" << prim <<
			    "With: r" << arg2 << " with:  ac" << ".\n";
			break;
		}

		/** a arg3, u8 prim-num, u8 arg1-reg, u8 arg2-reg */
		case Op::kPrimitive3: {
			unsigned prim = FETCH, arg1 = FETCH, arg2 = FETCH;
			std::cout << "self primitive " << prim <<
			    "With: r" << arg1 << " with:  r" << arg2 <<
			    "with: ac.\n";
			break;
		}

		/** u8 prim-num, u8 nArgs, u8 arg1-reg, */
		case Op::kPrimitiveV: {
			unsigned prim = FETCH, nArgs = FETCH, arg1reg = FETCH;
			std::cout << "self primitive" << prim << "WithNArgs:" <<
			    nArgs << " firstArgReg: r" << arg1reg << ".\n";
			break;
		}

		case Op::kReturnSelf: {
			std::cout << "self return: receiver.\n";
			break;
		}

		/* u8 source-register */
		case Op::kReturn: {
			std::cout << "self return: ac.\n";
			break;
		}

		case Op::kBlockReturn: {
			std::cout << "self blockReturn: ac.\n";
			break;
		}
		}
	}
}