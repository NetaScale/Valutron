#include <iostream>

#include "Interpreter.hh"

void
disassemble(uint8_t *code, int length)
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
			std::cout << "MoveParentHeapVar: " << src <<
			    " toMyHeapVar: " << dst << "\n";
			break;
		}

		case Op::kStoreToMyHeapVars: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "Store: r" << src << " toMyHeapVar: " <<
			    dst << "\n";
			break;
		}

		case Op::kMoveMyHeapVarToParentHeapVars: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "MoveMyHeapVar: " << src <<
			    " toParentHeapVar: " << dst << "\n";
			break;
		}

		/* u8 dest-reg */
		case Op::kLoadNil: {
			unsigned dst = FETCH;
			std::cout << "LoadNil: r" << dst <<"\n";
			break;
		}

		case Op::kLoadTrue: {
			unsigned dst = FETCH;
			std::cout << "LoadTrue: r" << dst <<"\n";
			break;
		}

		case Op::kLoadFalse: {
			unsigned dst = FETCH;
			std::cout << "LoadFalse: r" << dst <<"\n";
			break;
		}

		case Op::kLoadThisContext: {
			unsigned dst = FETCH;
			std::cout << "LoadThisContext: r" << dst <<"\n";
			break;
		}

		case Op::kLoadSmalltalk: {
			unsigned dst = FETCH;
			std::cout << "LoadSmalltalk: r" << dst <<"\n";
			break;
		}

		/* u8 index, u8 dest-reg */
		case Op::kLoadParentHeapVar: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "LoadParentHeapVar: " << src <<
			    " into: r" << dst << "\n";
			break;
		}

		case Op::kLoadMyHeapVar: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "LoadMyHeapVar: " << src <<
			    " into: r" << dst << "\n";
			break;
		}

		case Op::kLoadGlobal: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "LoadGlobal: l" << src <<
			    " into: r" << dst << "\n";
			break;
		}

		case Op::kLoadNstVar: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "LoadNstVar: " << src <<
			    " into: r" << dst << "\n";
			break;
		}

		case Op::kLoadLiteral: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "LoadLiteral: l" << src <<
			    " into: r" << dst << "\n";
			break;
		}

		/* u8 index, u8 source-reg */
		case Op::kStoreNstVar: {
			unsigned dst = FETCH, src = FETCH;
			std::cout << "Store: r" << src <<
			    " intoNstVar: " << dst << "\n";
			break;
		}

		case Op::kStoreGlobal: {
			unsigned dst = FETCH, src = FETCH;
			std::cout << "Store: r" << src <<
			    " intoGlobal: l" << dst << "\n";
			break;
		}

		case Op::kStoreParentHeapVar: {
			unsigned dst = FETCH, src = FETCH;
			std::cout << "Store: r" << src <<
			    " intoParentHeapVar: " << dst << "\n";
			break;
		}

		case Op::kStoreMyHeapVar: {
			unsigned dst = FETCH, src = FETCH;
			std::cout << "Store: r" << src <<
			    " intoMyHeapVar: " << dst << "\n";
			break;
		}

		case Op::kMove: {
			unsigned dst = FETCH, src = FETCH;
			std::cout << "Move: r" << src <<
			    " into: r" << dst << "\n";

			break;
		}

		/**
		 * u8 dest-reg, u8 receiver-reg, u8 selector-lit-idx,
		 * u8 num-args, (u8 arg-reg)+
		 */
		case Op::kSend: {
			unsigned dst = FETCH, rcvr = FETCH, selIdx = FETCH,
			    nArgs = FETCH;
			std::cout << "Send: r" << rcvr << " msg: l" << selIdx;

			if (nArgs > 0) {
				std::cout << " args: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << unsigned(FETCH);
				}
				std::cout << "]";
			}

			std::cout << " storeIn: r" << dst << "\n";

			break;
		}

		/**
		 * u8 dest-reg, u8 selector-literal-index, u8 num-args,
		 * (u8 arg-register)+
		 */
		case Op::kSendSuper: {
			unsigned dst = FETCH, selIdx = FETCH, nArgs = FETCH;
			std::cout << "SendSuperMsg: l" << selIdx;

			if (nArgs > 0) {
				std::cout << " args: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << FETCH;
				}
				std::cout << "]";
			}

			std::cout << " storeIn: r" << dst << "\n";
			break;
		}

		/** u8 dest-register, u8 prim-num, u8 num-args, (u8 arg-reg)+ */
		case Op::kPrimitive: {
			unsigned dst = FETCH, prim = FETCH, nArgs = FETCH;
			std::cout << "Primitive: " << prim;

			if (nArgs > 0) {
				std::cout << " args: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << FETCH;
				}
				std::cout << "]";
			}

			std::cout << " storeIn: r" << dst << "\n";
			break;
		}

		case Op::kReturnSelf: {
			std::cout << "ReturnSelf\n";
			break;
		}

		/* u8 source-register */
		case Op::kReturn: {
			unsigned src = FETCH;
			std::cout << "Return: r" << src <<"\n";
			break;
		}

		case Op::kBlockReturn: {
			unsigned src = FETCH;
			std::cout << "BlockReturn: r" << src <<"\n";
			break;
		}
		}
	}
}