#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <string>
#include <algorithm>
using namespace std;

void GenerateApp(string name, bool is_module);

int main(int argc, char* argv[])
{
	try
	{
		//Testear entrada
		if(argc!=3)
		{
			std::cout<<"XAPPGEN <Nombre de aplicacion> <Application/Module>"<<std::endl;
			return 0;
		}

		//Ejecutar el generador
		GenerateApp(argv[1], string(argv[2]) == "Module");
	}
	catch(std::string error)
	{
		std::cout<<error.c_str()<<std::endl;
	}
	catch(...)
	{
		std::cout<<"Unknown error"<<std::endl;
	}

	return 0;
}

void GenerateDIR(string name, bool is_module)
{
	char buffer[256];

	sprintf(buffer, "md %s", name.c_str());
	system(buffer);
	sprintf(buffer, "md %s\\Bin", name.c_str());
	system(buffer);
	sprintf(buffer, "md %s\\Bin\\Debug", name.c_str());
	system(buffer);
	sprintf(buffer, "md %s\\Bin\\Release", name.c_str());
	system(buffer);
	sprintf(buffer, "md %s\\Project", name.c_str());
	system(buffer);
	sprintf(buffer, "md %s\\Source", name.c_str());
	system(buffer);
	if(is_module)
	{
		sprintf(buffer, "md %s\\Exports", name.c_str());
		system(buffer);
	}
}

void GenerateVCPROJ(string name, bool is_module)
{
	string file_name = name + "\\Project\\" + name + ".vcproj";
	FILE* out = fopen(file_name.c_str(), "w");

	fprintf(out, "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>\n");
	fprintf(out, "<VisualStudioProject\n");
	fprintf(out, "	ProjectType=\"Visual C++\"\n");
	fprintf(out, "	Version=\"7.10\"\n");
	fprintf(out, "	Name=\"%s\"\n", name.c_str());
	fprintf(out, "	ProjectGUID=\"{19EC225B-67A3-428C-B69B-264F2C363A8C}\"\n");
	fprintf(out, "	Keyword=\"Win32Proj\">\n");
	fprintf(out, "	<Platforms>\n");
	fprintf(out, "		<Platform\n");
	fprintf(out, "			Name=\"Win32\"/>\n");
	fprintf(out, "	</Platforms>\n");
	fprintf(out, "	<Configurations>\n");
	fprintf(out, "		<Configuration\n");
	fprintf(out, "			Name=\"Debug|Win32\"\n");
	fprintf(out, "			OutputDirectory=\"..\\Bin\\Debug\"\n");
	fprintf(out, "			IntermediateDirectory=\"Debug\"\n");
	fprintf(out, "			ConfigurationType=\"2\"\n");
	fprintf(out, "			CharacterSet=\"0\">\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCCLCompilerTool\"\n");
	fprintf(out, "				Optimization=\"0\"\n");
	fprintf(out, "				AdditionalIncludeDirectories=\"..\\..\\..\\Inc\\Api;..\\..\\..\\Inc\\OS;..\\..\\..\\Api\\Exports\"\n");
	fprintf(out, "				MinimalRebuild=\"FALSE\"\n");
	fprintf(out, "				ExceptionHandling=\"FALSE\"\n");
	fprintf(out, "				BasicRuntimeChecks=\"0\"\n");
	fprintf(out, "				RuntimeLibrary=\"4\"\n");
	fprintf(out, "				StructMemberAlignment=\"1\"\n");
	fprintf(out, "				BufferSecurityCheck=\"FALSE\"\n");
	fprintf(out, "				UsePrecompiledHeader=\"0\"\n");
	fprintf(out, "				WarningLevel=\"4\"\n");
	fprintf(out, "				Detect64BitPortabilityProblems=\"FALSE\"\n");
	fprintf(out, "				DebugInformationFormat=\"0\"\n");
	fprintf(out, "				CallingConvention=\"2\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCCustomBuildTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCLinkerTool\"\n");
	fprintf(out, "				AdditionalOptions=\"/SUBSYSTEM:native\"\n");
	fprintf(out, "				OutputFile=\"$(OutDir)/%s.pe\"\n", name.c_str());
	fprintf(out, "				LinkIncremental=\"1\"\n");
	fprintf(out, "				IgnoreAllDefaultLibraries=\"TRUE\"\n");
	fprintf(out, "				IgnoreDefaultLibraryNames=\"kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib\"\n");
	fprintf(out, "				GenerateDebugInformation=\"FALSE\"\n");
	fprintf(out, "				ProgramDatabaseFile=\"\"\n");
	fprintf(out, "				SubSystem=\"0\"\n");
	fprintf(out, "				ResourceOnlyDLL=\"TRUE\"\n");
	fprintf(out, "				BaseAddress=\"0\"\n");
	fprintf(out, "				TargetMachine=\"1\"\n");
	fprintf(out, "				FixedBaseAddress=\"1\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCMIDLTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCPostBuildEventTool\"\n");
	fprintf(out, "				Description=\"Translating for XkyOS...\"\n");
	fprintf(out, "				CommandLine=\"copy ..\\PE2X.exe ..\\Bin\\Debug\n");
	fprintf(out, "cd ..\\Bin\\Debug\n");
	fprintf(out, "PE2X %s.pe %s.x\n", name.c_str(), name.c_str());
	fprintf(out, "del PE2X.exe\n");
	fprintf(out, "cd ..\\..\\Project\n");
	fprintf(out, "\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCPreBuildEventTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCPreLinkEventTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCResourceCompilerTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCWebServiceProxyGeneratorTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCXMLDataGeneratorTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCWebDeploymentTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCManagedWrapperGeneratorTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>\n");
	fprintf(out, "		</Configuration>\n");
	fprintf(out, "		<Configuration\n");
	fprintf(out, "			Name=\"Release|Win32\"\n");
	fprintf(out, "			OutputDirectory=\"..\\Bin\\Release\"\n");
	fprintf(out, "			IntermediateDirectory=\"Release\"\n");
	fprintf(out, "			ConfigurationType=\"2\"\n");
	fprintf(out, "			CharacterSet=\"0\">\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCCLCompilerTool\"\n");
	fprintf(out, "				AdditionalIncludeDirectories=\"..\\..\\..\\Inc\\Api;..\\..\\..\\Inc\\OS;..\\..\\..\\Api\\Exports\"\n");
	fprintf(out, "				MinimalRebuild=\"FALSE\"\n");
	fprintf(out, "				ExceptionHandling=\"FALSE\"\n");
	fprintf(out, "				BasicRuntimeChecks=\"0\"\n");
	fprintf(out, "				RuntimeLibrary=\"4\"\n");
	fprintf(out, "				StructMemberAlignment=\"1\"\n");
	fprintf(out, "				BufferSecurityCheck=\"FALSE\"\n");
	fprintf(out, "				UsePrecompiledHeader=\"0\"\n");
	fprintf(out, "				WarningLevel=\"4\"\n");
	fprintf(out, "				Detect64BitPortabilityProblems=\"FALSE\"\n");
	fprintf(out, "				DebugInformationFormat=\"0\"\n");
	fprintf(out, "				CallingConvention=\"2\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCCustomBuildTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCLinkerTool\"\n");
	fprintf(out, "				AdditionalOptions=\"/SUBSYSTEM:native\"\n");
	fprintf(out, "				OutputFile=\"$(OutDir)/%s.pe\"\n", name.c_str());
	fprintf(out, "				LinkIncremental=\"1\"\n");
	fprintf(out, "				IgnoreAllDefaultLibraries=\"TRUE\"\n");
	fprintf(out, "				IgnoreDefaultLibraryNames=\"kernel32.lib;user32.lib;gdi32.lib;winspool.lib;comdlg32.lib;advapi32.lib;shell32.lib;ole32.lib;oleaut32.lib;uuid.lib;odbc32.lib;odbccp32.lib\"\n");
	fprintf(out, "				GenerateDebugInformation=\"FALSE\"\n");
	fprintf(out, "				SubSystem=\"0\"\n");
	fprintf(out, "				OptimizeReferences=\"2\"\n");
	fprintf(out, "				EnableCOMDATFolding=\"2\"\n");
	fprintf(out, "				ResourceOnlyDLL=\"TRUE\"\n");
	fprintf(out, "				BaseAddress=\"0\"\n");
	fprintf(out, "				TargetMachine=\"1\"\n");
	fprintf(out, "				FixedBaseAddress=\"1\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCMIDLTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCPostBuildEventTool\"\n");
	fprintf(out, "				Description=\"Translating for XkyOS...\"\n");
	fprintf(out, "				CommandLine=\"copy ..\\PE2X.exe ..\\Bin\\Release\n");
	fprintf(out, "cd ..\\Bin\\Release\n");
	fprintf(out, "PE2X %s.pe %s.x\n", name.c_str(), name.c_str());
	fprintf(out, "del PE2X.exe\n");
	fprintf(out, "cd ..\\..\\Project\n");
	fprintf(out, "\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCPreBuildEventTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCPreLinkEventTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCResourceCompilerTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCWebServiceProxyGeneratorTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCXMLDataGeneratorTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCWebDeploymentTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCManagedWrapperGeneratorTool\"/>\n");
	fprintf(out, "			<Tool\n");
	fprintf(out, "				Name=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>\n");
	fprintf(out, "		</Configuration>\n");
	fprintf(out, "	</Configurations>\n");
	fprintf(out, "	<References>\n");
	fprintf(out, "	</References>\n");
	fprintf(out, "	<Files>\n");
	fprintf(out, "		<Filter\n");
	fprintf(out, "			Name=\"Source\"\n");
	fprintf(out, "			Filter=\"\">\n");
	fprintf(out, "			<File\n");
	fprintf(out, "				RelativePath=\"..\\Source\\%s.cpp\">\n", name.c_str());
	fprintf(out, "			</File>\n");
	fprintf(out, "		</Filter>\n");
	fprintf(out, "		<Filter\n");
	fprintf(out, "			Name=\"Imports\"\n");
	fprintf(out, "			Filter=\"\">\n");
	fprintf(out, "			<File\n");
	fprintf(out, "				RelativePath=\"..\\..\\..\\INC\\API\\API.h\">\n");
	fprintf(out, "			</File>\n");
	fprintf(out, "		</Filter>\n");
	fprintf(out, "		<Filter\n");
	fprintf(out, "			Name=\"OS\"\n");
	fprintf(out, "			Filter=\"\">\n");
	fprintf(out, "			<File\n");
	fprintf(out, "				RelativePath=\"..\\..\\..\\Inc\\OS\\Executable.h\">\n");
	fprintf(out, "			</File>\n");
	fprintf(out, "			<File\n");
	fprintf(out, "				RelativePath=\"..\\..\\..\\Inc\\OS\\Image.h\">\n");
	fprintf(out, "			</File>\n");
	fprintf(out, "			<File\n");
	fprintf(out, "				RelativePath=\"..\\..\\..\\Inc\\OS\\Types.h\">\n");
	fprintf(out, "			</File>\n");
	fprintf(out, "		</Filter>\n");
	if(is_module)
	{
		fprintf(out, "		<Filter\n");
		fprintf(out, "			Name=\"Exports\"\n");
		fprintf(out, "			Filter=\"\">\n");
		fprintf(out, "			<File\n");
		fprintf(out, "				RelativePath=\"..\\Exports\\%s.h\">\n", name.c_str());
		fprintf(out, "			</File>\n");
		fprintf(out, "		</Filter>\n");
	}
	fprintf(out, "	</Files>\n");
	fprintf(out, "	<Globals>\n");
	fprintf(out, "	</Globals>\n");
	fprintf(out, "</VisualStudioProject>\n");

	fclose(out);
}

void GenerateSLN(string name)
{
	string file_name = name + "\\Project\\" + name + ".sln";
	FILE* out = fopen(file_name.c_str(), "w");

	fprintf(out, "Microsoft Visual Studio Solution File, Format Version 8.00\n");
	fprintf(out, "Project(\"{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\") = \"%s\", \"%s.vcproj\", \"{19EC225B-67A3-428C-B69B-264F2C363A8C}\"\n", name.c_str(), name.c_str());
	fprintf(out, "	ProjectSection(ProjectDependencies) = postProject\n");
	fprintf(out, "	EndProjectSection\n");
	fprintf(out, "EndProject\n");
	fprintf(out, "Global\n");
	fprintf(out, "	GlobalSection(DPCodeReviewSolutionGUID) = preSolution\n");
	fprintf(out, "		DPCodeReviewSolutionGUID = {00000000-0000-0000-0000-000000000000}\n");
	fprintf(out, "	EndGlobalSection\n");
	fprintf(out, "	GlobalSection(SolutionConfiguration) = preSolution\n");
	fprintf(out, "		Debug = Debug\n");
	fprintf(out, "		Release = Release\n");
	fprintf(out, "	EndGlobalSection\n");
	fprintf(out, "	GlobalSection(ProjectDependencies) = postSolution\n");
	fprintf(out, "	EndGlobalSection\n");
	fprintf(out, "	GlobalSection(ProjectConfiguration) = postSolution\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Debug.ActiveCfg = Debug|Win32\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Debug.Build.0 = Debug|Win32\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Release.ActiveCfg = Release|Win32\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Release.Build.0 = Release|Win32\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Debug.ActiveCfg = Debug|Win32\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Debug.Build.0 = Debug|Win32\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Release.ActiveCfg = Release|Win32\n");
	fprintf(out, "		{19EC225B-67A3-428C-B69B-264F2C363A8C}.Release.Build.0 = Release|Win32\n");
	fprintf(out, "	EndGlobalSection\n");
	fprintf(out, "	GlobalSection(ExtensibilityGlobals) = postSolution\n");
	fprintf(out, "	EndGlobalSection\n");
	fprintf(out, "	GlobalSection(ExtensibilityAddIns) = postSolution\n");
	fprintf(out, "	EndGlobalSection\n");
	fprintf(out, "EndGlobal\n");

	fclose(out);
}

void GenerateDOXYGEN(string name)
{
	string file_name = name + "\\Project\\Doxyfile.cfg";
	FILE* out = fopen(file_name.c_str(), "w");

	fprintf(out, "PROJECT_NUMBER = 1\n");
	fprintf(out, "OUTPUT_DIRECTORY = Doc\n");
	fprintf(out, "EXTRACT_ALL = NO\n");
	fprintf(out, "EXTRACT_STATIC = YES\n");
	fprintf(out, "EXTRACT_LOCAL_CLASSES = YES\n");
	fprintf(out, "BRIEF_MEMBER_DESC = YES\n");
	fprintf(out, "REPEAT_BRIEF = YES\n");
	fprintf(out, "ALWAYS_DETAILED_SEC = YES\n");
	fprintf(out, "STRIP_FROM_PATH =\n");
	fprintf(out, "STRIP_CODE_COMMENTS = YES\n");
	fprintf(out, "CASE_SENSE_NAMES = YES\n");
	fprintf(out, "SHORT_NAMES = NO\n");
	fprintf(out, "HIDE_SCOPE_NAMES = NO\n");
	fprintf(out, "JAVADOC_AUTOBRIEF = NO\n");
	fprintf(out, "INHERIT_DOCS = YES\n");
	fprintf(out, "INLINE_INFO = YES\n");
	fprintf(out, "DISTRIBUTE_GROUP_DOC = NO\n");
	fprintf(out, "GENERATE_TESTLIST = NO\n");
	fprintf(out, "ALIASES =\n");
	fprintf(out, "ENABLED_SECTIONS =\n");
	fprintf(out, "MAX_INITIALIZER_LINES = 10\n");
	fprintf(out, "OPTIMIZE_OUTPUT_FOR_C = NO\n");
	fprintf(out, "OPTIMIZE_OUTPUT_JAVA = NO\n");
	fprintf(out, "SHOW_USED_FILES = NO\n");
	fprintf(out, "QUIET = NO\n");
	fprintf(out, "WARNINGS = YES\n");
	fprintf(out, "WARN_IF_UNDOCUMENTED = NO\n");
	fprintf(out, "WARN_FORMAT = \"$file($line) $text\"\n");
	fprintf(out, "WARN_LOGFILE =\n");
	fprintf(out, "FILE_PATTERNS =\n");
	fprintf(out, "RECURSIVE = NO\n");
	fprintf(out, "EXCLUDE =\n");
	fprintf(out, "EXCLUDE_SYMLINKS = NO\n");
	fprintf(out, "EXCLUDE_PATTERNS =\n");
	fprintf(out, "EXAMPLE_PATH = .\n");
	fprintf(out, "EXAMPLE_PATTERNS =\n");
	fprintf(out, "EXAMPLE_RECURSIVE = YES\n");
	fprintf(out, "INPUT_FILTER =\n");
	fprintf(out, "FILTER_SOURCE_FILES = NO\n");
	fprintf(out, "ALPHABETICAL_INDEX = YES\n");
	fprintf(out, "COLS_IN_ALPHA_INDEX = 5\n");
	fprintf(out, "IGNORE_PREFIX =\n");
	fprintf(out, "HTML_OUTPUT =\n");
	fprintf(out, "HTML_FILE_EXTENSION =\n");
	fprintf(out, "HTML_HEADER =\n");
	fprintf(out, "HTML_FOOTER =\n");
	fprintf(out, "HTML_STYLESHEET =\n");
	fprintf(out, "HTML_ALIGN_MEMBERS = YES\n");
	fprintf(out, "BINARY_TOC = NO\n");
	fprintf(out, "TOC_EXPAND = NO\n");
	fprintf(out, "DISABLE_INDEX = YES\n");
	fprintf(out, "ENUM_VALUES_PER_LINE = 4\n");
	fprintf(out, "GENERATE_TREEVIEW = YES\n");
	fprintf(out, "TREEVIEW_WIDTH = 250\n");
	fprintf(out, "LATEX_OUTPUT =\n");
	fprintf(out, "MAKEINDEX_CMD_NAME =\n");
	fprintf(out, "COMPACT_LATEX = NO\n");
	fprintf(out, "PAPER_TYPE = a4wide\n");
	fprintf(out, "EXTRA_PACKAGES =\n");
	fprintf(out, "LATEX_HEADER =\n");
	fprintf(out, "PDF_HYPERLINKS = YES\n");
	fprintf(out, "USE_PDFLATEX = YES\n");
	fprintf(out, "LATEX_BATCHMODE = YES\n");
	fprintf(out, "RTF_OUTPUT =\n");
	fprintf(out, "COMPACT_RTF = NO\n");
	fprintf(out, "RTF_HYPERLINKS = YES\n");
	fprintf(out, "RTF_STYLESHEET_FILE =\n");
	fprintf(out, "RTF_EXTENSIONS_FILE =\n");
	fprintf(out, "GENERATE_MAN = NO\n");
	fprintf(out, "MAN_OUTPUT =\n");
	fprintf(out, "MAN_EXTENSION = .3\n");
	fprintf(out, "MAN_LINKS = YES\n");
	fprintf(out, "GENERATE_AUTOGEN_DEF = NO\n");
	fprintf(out, "ENABLE_PREPROCESSING = YES\n");
	fprintf(out, "MACRO_EXPANSION = NO\n");
	fprintf(out, "EXPAND_ONLY_PREDEF = NO\n");
	fprintf(out, "SEARCH_INCLUDES = YES\n");
	fprintf(out, "INCLUDE_PATH =\n");
	fprintf(out, "INCLUDE_FILE_PATTERNS =\n");
	fprintf(out, "PREDEFINED = \"DECLARE_INTERFACE(name)=class name\" \\ \n");
	fprintf(out, "\"STDMETHOD(result,name)=virtual result name\" \\ \n");
	fprintf(out, "\"PURE= = 0\" \\ \n");
	fprintf(out, "THIS_= \\ \n");
	fprintf(out, "THIS= \\ \n");
	fprintf(out, "DECLARE_REGISTRY_RESOURCEID=// \\ \n");
	fprintf(out, "DECLARE_PROTECT_FINAL_CONSTRUCT=// \\ \n");
	fprintf(out, "\"DECLARE_AGGREGATABLE(Class)= \" \\ \n");
	fprintf(out, "\"DECLARE_REGISTRY_RESOURCEID(Id)= \" \\ \n");
	fprintf(out, "DECLARE_MESSAGE_MAP = \\ \n");
	fprintf(out, "BEGIN_MESSAGE_MAP=/* \\ \n");
	fprintf(out, "END_MESSAGE_MAP=*/// \\ \n");
	fprintf(out, "BEGIN_COM_MAP=/* \\ \n");
	fprintf(out, "END_COM_MAP=*/// \\ \n");
	fprintf(out, "BEGIN_PROP_MAP=/* \\ \n");
	fprintf(out, "END_PROP_MAP=*/// \\ \n");
	fprintf(out, "BEGIN_MSG_MAP=/* \\ \n");
	fprintf(out, "END_MSG_MAP=*/// \\ \n");
	fprintf(out, "BEGIN_PROPERTY_MAP=/* \\ \n");
	fprintf(out, "END_PROPERTY_MAP=*/// \\ \n");
	fprintf(out, "BEGIN_OBJECT_MAP=/* \\ \n");
	fprintf(out, "END_OBJECT_MAP()=*/// \\ \n");
	fprintf(out, "DECLARE_VIEW_STATUS=// \\ \n");
	fprintf(out, "\"STDMETHOD(a)=HRESULT a\" \\ \n");
	fprintf(out, "\"ATL_NO_VTABLE= \" \\ \n");
	fprintf(out, "\"__declspec(a)= \" \\ \n");
	fprintf(out, "BEGIN_CONNECTION_POINT_MAP=/* \\ \n");
	fprintf(out, "END_CONNECTION_POINT_MAP=*/// \\ \n");
	fprintf(out, "\"DECLARE_DYNAMIC(class)= \" \\ \n");
	fprintf(out, "\"IMPLEMENT_DYNAMIC(class1, class2)= \" \\ \n");
	fprintf(out, "\"DECLARE_DYNCREATE(class)= \" \\ \n");
	fprintf(out, "\"IMPLEMENT_DYNCREATE(class1, class2)= \" \\ \n");
	fprintf(out, "\"IMPLEMENT_SERIAL(class1, class2, class3)= \" \\ \n");
	fprintf(out, "\"DECLARE_MESSAGE_MAP()= \" \\ \n");
	fprintf(out, "TRY=try \\ \n");
	fprintf(out, "\"CATCH_ALL(e)= catch(...)\" \\ \n");
	fprintf(out, "END_CATCH_ALL= \\ \n");
	fprintf(out, "\"THROW_LAST()= throw\"\\ \n");
	fprintf(out, "\"RUNTIME_CLASS(class)=class\" \\ \n");
	fprintf(out, "\"MAKEINTRESOURCE(nId)=nId\" \\ \n");
	fprintf(out, "\"IMPLEMENT_REGISTER(v, w, x, y, z)= \" \\ \n");
	fprintf(out, "\"ASSERT(x)=assert(x)\" \\ \n");
	fprintf(out, "\"ASSERT_VALID(x)=assert(x)\" \\ \n");
	fprintf(out, "\"TRACE0(x)=printf(x)\" \\ \n");
	fprintf(out, "\"OS_ERR(A,B)={ #A, B }\" \\ \n");
	fprintf(out, "__cplusplus \\ \n");
	fprintf(out, "\"DECLARE_OLECREATE(class)= \" \\ \n");
	fprintf(out, "\"BEGIN_DISPATCH_MAP(class1, class2)= \" \\ \n");
	fprintf(out, "\"INTERFACE_PART(class, id, name)= \" \\ \n");
	fprintf(out, "\"END_INTERFACE_MAP()=\" \\ \n");
	fprintf(out, "\"DISP_FUNCTION(class, name, function, result, id)=\" \\ \n");
	fprintf(out, "\"END_DISPATCH_MAP()=\" \\ \n");
	fprintf(out, "\"IMPLEMENT_OLECREATE2(class, name, id1, id2, id3, id4, id5, id6, id7, id8, id9, id10, id11)=\"\n");
	fprintf(out, "EXPAND_AS_DEFINED =\n");
	fprintf(out, "SKIP_FUNCTION_MACROS =\n");
	fprintf(out, "TAGFILES =\n");
	fprintf(out, "GENERATE_TAGFILE =\n");
	fprintf(out, "ALLEXTERNALS = NO\n");
	fprintf(out, "EXTERNAL_GROUPS = NO\n");
	fprintf(out, "PERL_PATH =\n");
	fprintf(out, "CLASS_DIAGRAMS = YES\n");
	fprintf(out, "HAVE_DOT = YES\n");
	fprintf(out, "CLASS_GRAPH = YES\n");
	fprintf(out, "COLLABORATION_GRAPH = YES\n");
	fprintf(out, "TEMPLATE_RELATIONS = YES\n");
	fprintf(out, "HIDE_UNDOC_RELATIONS = NO\n");
	fprintf(out, "INCLUDE_GRAPH = YES\n");
	fprintf(out, "INCLUDED_BY_GRAPH = YES\n");
	fprintf(out, "GRAPHICAL_HIERARCHY = YES\n");
	fprintf(out, "DOT_IMAGE_FORMAT = png\n");
	fprintf(out, "DOTFILE_DIRS =\n");
	fprintf(out, "MAX_DOT_GRAPH_WIDTH =\n");
	fprintf(out, "MAX_DOT_GRAPH_HEIGHT =\n");
	fprintf(out, "GENERATE_LEGEND = YES\n");
	fprintf(out, "DOT_CLEANUP = YES\n");
	fprintf(out, "SEARCHENGINE = NO\n");

	fclose(out);
}


void GenerateCODE(string name, bool is_module)
{
	string cpp_file_name = name + "\\Source\\" + name + ".cpp";
	FILE* cpp_out = fopen(cpp_file_name.c_str(), "w");

	fprintf(cpp_out, "/******************************************************************************/\n");
	fprintf(cpp_out, "/**\n");
	fprintf(cpp_out, "* @file	%s.cpp\n", name.c_str());
	fprintf(cpp_out, "* @brief	PUT HERE YOUR SHORT DESCRIPTION\n");
	fprintf(cpp_out, "* PUT HERE YOUR LONG DESCRIPTION\n");
	fprintf(cpp_out, "*\n");
	fprintf(cpp_out, "* @date	PUT HERE THE DATE\n");
	fprintf(cpp_out, "* @author	PUT HERE YOUR NAME\n");
	fprintf(cpp_out, "*/\n");
	fprintf(cpp_out, "/******************************************************************************/\n");
	fprintf(cpp_out, "#include \"Types.h\"\n");
	fprintf(cpp_out, "#include \"Executable.h\"\n");
	fprintf(cpp_out, "\n");
	fprintf(cpp_out, "//PUT HERE YOUR ADDITONAL INCLUDES\n");
	fprintf(cpp_out, "\n");
	fprintf(cpp_out, "//=================================IMPORTS====================================//\n");
	fprintf(cpp_out, "#pragma data_seg(\".imports\")\n");
	fprintf(cpp_out, "//============================================================================//\n");
	fprintf(cpp_out, "#include \"API.h\"\n");
	fprintf(cpp_out, "\n");
	fprintf(cpp_out, "//==================================DATA======================================//\n");
	fprintf(cpp_out, "#pragma data_seg(\".data\")\n");
	fprintf(cpp_out, "//============================================================================//\n");
	fprintf(cpp_out, "//PUT HERE YOUR GLOBAL VARIABLES\n");
	fprintf(cpp_out, "\n");
	fprintf(cpp_out, "//==================================CODE======================================//\n");
	fprintf(cpp_out, "#pragma code_seg(\".code\")\n");
	fprintf(cpp_out, "//============================================================================//\n");
	fprintf(cpp_out, "//PUT HERE YOUR CODE\n");
	fprintf(cpp_out, "\n");
	if(is_module)
	{
		fprintf(cpp_out, "/*\n");
		fprintf(cpp_out, "PUBLIC dword MyExportedFunction(IN int _parameter)\n");
		fprintf(cpp_out, "{\n");
		fprintf(cpp_out, "	return 42;\n");
		fprintf(cpp_out, "}\n");
		fprintf(cpp_out, "*/\n");
	}
	else
	{
		fprintf(cpp_out, "PUBLIC void Main()\n");
		fprintf(cpp_out, "{\n");
		fprintf(cpp_out, "	//PUT HERE YOUR CODE\n");
		fprintf(cpp_out, "	//string hello_world = STRING(\"Hello world!\");\n");
		fprintf(cpp_out, "	//XKY_DEBUG_Message(&hello_world, SRGB(255, 0, 0));\n");
		fprintf(cpp_out, "	XKY_OS_Finish();\n");
		fprintf(cpp_out, "}\n");
	}
	fprintf(cpp_out, "\n");
	fprintf(cpp_out, "//=================================EXPORTS====================================//\n");
	fprintf(cpp_out, "#pragma data_seg(\".exports\")\n");
	fprintf(cpp_out, "//============================================================================//\n");
	if(is_module)
	{
		fprintf(cpp_out, "//EXPORT(MyExportedFunction)\n");
	}
	fprintf(cpp_out, "\n");
	fprintf(cpp_out, "//=================================MODULE=====================================//\n");
	fprintf(cpp_out, "#pragma data_seg(\".module\")\n");
	fprintf(cpp_out, "//============================================================================//\n");
	if(is_module)
	{
		fprintf(cpp_out, "	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, 0, 0);\n");
	}
	else
	{
		fprintf(cpp_out, "	MODULE(IMAGE_MODE_USER, IMAGE_KIND_MODULE, IMAGE_VERSION(1,0,0,0), 0, Main, 0);\n");
	}
	fclose(cpp_out);

	if(is_module)
	{
		string h_file_name = name + "\\Exports\\" + name + ".h";
		FILE* h_out = fopen(h_file_name.c_str(), "w");

		fprintf(h_out, "/******************************************************************************/\n");
		fprintf(h_out, "/**\n");
		fprintf(h_out, "* @file		%s.h\n", name.c_str());
		fprintf(h_out, "* @brief	PUT HERE YOUR SHORT DESCRIPTION\n");
		fprintf(h_out, "* PUT HERE YOUR LONG DESCRIPTION\n");
		fprintf(h_out, "*\n");
		fprintf(h_out, "* @date		PUT HERE THE DATE\n");
		fprintf(h_out, "* @author	PUT HERE YOUR NAME\n");
		fprintf(h_out, "*/\n");
		fprintf(h_out, "/******************************************************************************/\n");
		transform(name.begin(), name.end(), name.begin(), toupper);
		fprintf(h_out, "#ifndef __%s_H__\n", name.c_str());
		fprintf(h_out, "#define __%s_H__\n", name.c_str());
		fprintf(h_out, "\n");
		fprintf(h_out, "//PUT HERE YOUR EXPORTED FUNCTION TYPES\n");
		fprintf(h_out, "\n");
		fprintf(h_out, "//typedef dword (*fMyExportedFunction)(IN int _parameter)\n");
		fprintf(h_out, "\n");
		fprintf(h_out, "#endif //__%s_H__\n", name.c_str());
		fclose(h_out);
	}
}

void GenerateApp(string name, bool is_module)
{
	GenerateDIR(name, is_module);
	GenerateSLN(name);
	GenerateVCPROJ(name, is_module);
	GenerateDOXYGEN(name);
	GenerateCODE(name, is_module);
}
