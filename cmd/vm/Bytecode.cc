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

		case Op::kMoveMyHeapVarToParentHeapVars: {
			unsigned src = FETCH, dst = FETCH;
			std::cout << "MoveMyHeapVar: " << src <<
			    " toParentHeapVar: " << dst << "\n";
			break;
		}

		case Op::kLdaNil:
			std::cout << "LdaNil\n";
			break;

		case Op::kLdaTrue:
			std::cout << "LdaTrue\n";
			break;

		case Op::kLdaFalse:
			std::cout << "LdaFalse\n";
			break;

		case Op::kLdaThisContext:
			std::cout << "LdaThisContext\n";
			break;

		case Op::kLdaSmalltalk:
			std::cout << "LdaSmalltalk\n";
			break;

		/* u8 index*/
		case Op::kLdaParentHeapVar: {
			unsigned src = FETCH;
			std::cout << "LdaParentHeapVar: " << src;
			break;
		}

		case Op::kLdaMyHeapVar: {
			unsigned src = FETCH;
			std::cout << "LdaMyHeapVar: " << src << "\n";
			break;
		}

		case Op::kLdaGlobal: {
			unsigned src = FETCH;
			std::cout << "LdaGlobal: l" << src << "\n";
			break;
		}

		case Op::kLdaNstVar: {
			unsigned src = FETCH;
			std::cout << "LdaNstVar: " << src << "\n";
			break;
		}

		case Op::kLdaLiteral: {
			unsigned src = FETCH;
			std::cout << "LdaLiteral: l" << src << "\n";
			break;
		}

		case Op::kLdaBlockCopy: {
			unsigned src = FETCH;
			std::cout << "LdaBlockCopy: l" << src << "\n";
			break;
		}

		case Op::kLdar: {
			unsigned src = FETCH;
			std::cout << "Ldar: r" << src << "\n";
			break;
		}

		/* u8 index */
		case Op::kStaNstVar: {
			unsigned dst = FETCH;
			std::cout << "StoreNstVar: " << dst << "\n";
			break;
		}

		case Op::kStaGlobal: {
			unsigned dst = FETCH;
			std::cout << "StoreGlobal: l" << dst << "\n";
			break;
		}

		case Op::kStaParentHeapVar: {
			unsigned dst = FETCH;
			std::cout << "StaParentHeapVar: " << dst << "\n";
			break;
		}

		case Op::kStaMyHeapVar: {
			unsigned dst = FETCH;
			std::cout << "StaMyHeapVar: " << dst << "\n";
			break;
		}

		case Op::kStar: {
			unsigned src = FETCH;
			std::cout << "Star: r" << src << "\n";
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
			unsigned selIdx = FETCH, nArgs = FETCH;
			std::cout << "SendMsg: l" << selIdx;

			if (nArgs > 0) {
				std::cout << " args: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << unsigned(FETCH);
				}
				std::cout << "]";
			}

			std::cout << "\n";

			break;
		}

		/**
		 * u8 dest-reg, u8 selector-literal-index, u8 num-args,
		 * (u8 arg-register)+
		 */
		case Op::kSendSuper: {
			unsigned selIdx = FETCH, nArgs = FETCH;
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

			std::cout << "\n";
			break;
		}

		/** u8 prim-num, u8 num-args, (u8 arg-reg)+ */
		case Op::kPrimitive: {
			unsigned prim = FETCH, nArgs = FETCH;
			std::cout << "Primitive: " << prim;

			if (nArgs > 0) {
				std::cout << " args: [";
				for (int i = 0; i < nArgs; i++) {
					if (i > 0)
						std::cout << ",";
					std::cout << "r" << (unsigned)FETCH;
				}
				std::cout << "]";
			}

			std::cout << "\n";
			break;
		}

		case Op::kReturnSelf: {
			std::cout << "ReturnSelf\n";
			break;
		}

		/* u8 source-register */
		case Op::kReturn: {
			std::cout << "ReturnA\n";
			break;
		}

		case Op::kBlockReturn: {
			std::cout << "BlockReturnA\n";
			break;
		}
		}
	}
}