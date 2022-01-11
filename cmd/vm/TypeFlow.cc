#include "AST.hh"
#include "Typecheck.hh"

std::vector<FlowInference>
MessageExprNode::makeFlowInferences(TyChecker &tyc)
{
	if (receiver->isIdent()) {
		IdentExprNode *object = dynamic_cast<IdentExprNode *>(receiver);

		if (selector == "isKindOf:" && args[0]->isIdent()) {
			Type *type = tyc.env()->lookupType(
			    dynamic_cast<IdentExprNode *>(args[0])->id);

			return { { false, false, object->id, type } };
		}
	}

	return {};
}