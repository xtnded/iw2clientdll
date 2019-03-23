#include <stdio.h>
#include "chakracore.h"

int main() {
	chakracore_prepare(printf);
	chakracore_eval("console.log(\"^1hai\");");
	chakracore_eval("console.log(1);");
	chakracore_eval("console.log(2);");
	chakracore_eval("console.log(3);");
	chakracore_eval("console.log(4);");
	chakracore_eval("console.log(5);");
	chakracore_eval("a = 1");
	chakracore_eval("b = 2");
	chakracore_eval("c = a + b");
	chakracore_eval("console.log(\"c is\", c)");
	getchar();
}
