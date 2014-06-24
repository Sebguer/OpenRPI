#include <string.h>


class DynamicVariable
{
public:
	DynamicVariable::DynamicVariable(std::string Name, std::string Contents)
	{
		this.VariableName = Name;
		this.VariableContents = Contents;
	}
	DynamicVariable::DynamicVariable(std::string Name)
	{
		this.VariableName = Name;
		this.VariableContents = "";
	}
	DynamicVariable::~DynamicVariable()
	{
	}
	std::string DynamicVariable::GetVariableName()
	{
		return this.VariableName;
	}
	std::string DynamicVariable::GetVariableContents()
	{
		return this.VariableContents;
	}
	void DynamicVariable::SetVariableContents(std::string Contents)
	{
		this.VariableContents.Assign(Contents);
	}
	void DynamicVariable::SetVariableName(std::string Name)
	{
		this.VariableName.Assign(Name);
	}
private:
	std::string VariableName;
	std::string VariableContents;
}

class DynamicVariableList : Public list<DynamicVariable>
{
public:
	DynamicVariable *GetNamedVariable(std::string Name)
	{
		if (this.empty() || Name.empty())
			return NULL;

		std::list<DynamicVariable>::iterator Member = this.begin();
		while (*Member != this.end() && Member->GetVariableName() != Name)
		{
			Member++;
		}

		if (*Member == this.end() && this.end()->GetVariableName() != Name)
			return NULL;

		return Member;
	}
private:

}
