"""
Test lldb data formatter subsystem.
"""

import lldb
from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil


class TestDataFormatterGenericForwardList(TestBase):
    def setUp(self):
        TestBase.setUp(self)
        self.line = line_number("main.cpp", "// break here")
        self.namespace = "std"

    def do_test(self):
        """Test that std::forward_list is displayed correctly"""
        lldbutil.run_to_source_breakpoint(
            self, "// break here", lldb.SBFileSpec("main.cpp", False)
        )

        forward_list = self.namespace + "::forward_list"
        self.expect("frame variable empty", substrs=[forward_list, "size=0", "{}"])

        self.expect(
            "frame variable one_elt",
            substrs=[forward_list, "size=1", "{", "[0] = 47", "}"],
        )

        self.expect(
            "frame variable five_elts",
            substrs=[
                forward_list,
                "size=5",
                "{",
                "[0] = 1",
                "[1] = 22",
                "[2] = 333",
                "[3] = 4444",
                "[4] = 55555",
                "}",
            ],
        )

        self.expect(
            "settings show target.max-children-count",
            matching=True,
            substrs=["target.max-children-count (unsigned) = 24"],
        )

        self.expect(
            "frame variable thousand_elts",
            matching=False,
            substrs=["[256]", "[333]", "[444]", "[555]", "[666]", "..."],
        )
        self.runCmd("settings set target.max-children-count 3", check=False)

        self.expect(
            "frame variable thousand_elts",
            matching=False,
            substrs=[
                "[3]",
                "[4]",
                "[5]",
            ],
        )

        self.expect(
            "frame variable thousand_elts",
            matching=True,
            substrs=["size=24", "[0]", "[1]", "[2]", "..."],
        )

    def do_test_ptr_and_ref(self):
        """Test that ref and ptr to std::forward_list is displayed correctly"""
        (_, process, _, bkpt) = lldbutil.run_to_source_breakpoint(
            self, "Check ref and ptr", lldb.SBFileSpec("main.cpp", False)
        )

        self.expect("frame variable ref", substrs=["size=0", "{}"])

        self.expect("frame variable *ptr", substrs=["size=0", "{}"])

        lldbutil.continue_to_breakpoint(process, bkpt)

        self.expect("frame variable ref", substrs=["{", "[0] = 47", "}"])

        self.expect("frame variable *ptr", substrs=["{", "[0] = 47", "}"])

        lldbutil.continue_to_breakpoint(process, bkpt)

        self.expect(
            "frame variable ref",
            substrs=[
                "size=5",
                "{",
                "[0] = 1",
                "[1] = 22",
                "[2] = 333",
                "[3] = 4444",
                "[4] = 55555",
                "}",
            ],
        )

        self.expect(
            "frame variable *ptr",
            substrs=[
                "size=5",
                "{",
                "[0] = 1",
                "[1] = 22",
                "[2] = 333",
                "[3] = 4444",
                "[4] = 55555",
                "}",
            ],
        )

        lldbutil.continue_to_breakpoint(process, bkpt)

        self.runCmd("settings set target.max-children-count 256", check=False)

        self.expect(
            "settings show target.max-children-count",
            matching=True,
            substrs=["target.max-children-count (unsigned) = 256"],
        )

        self.expect(
            "frame variable ref",
            matching=True,
            substrs=[
                "size=256",
                "[0] = 999",
                "[1] = 998",
                "[2] = 997",
            ],
        )

        self.expect(
            "frame variable *ptr",
            matching=True,
            substrs=[
                "size=256",
                "[0] = 999",
                "[1] = 998",
                "[2] = 997",
            ],
        )

    @add_test_categories(["libstdcxx"])
    def test_libstdcpp(self):
        self.build(dictionary={"USE_LIBSTDCPP": 1})
        self.do_test()

    @add_test_categories(["libstdcxx"])
    def test_ptr_and_ref_libstdcpp(self):
        self.build(dictionary={"USE_LIBSTDCPP": 1})
        self.do_test_ptr_and_ref()

    @add_test_categories(["libc++"])
    def test_libcpp(self):
        self.build(dictionary={"USE_LIBCPP": 1})
        self.do_test()

    @add_test_categories(["libc++"])
    def test_ptr_and_ref_libcpp(self):
        self.build(dictionary={"USE_LIBCPP": 1})
        self.do_test_ptr_and_ref()

    @add_test_categories(["msvcstl"])
    def test_msvcstl(self):
        # No flags, because the "msvcstl" category checks that the MSVC STL is used by default.
        self.build()
        self.do_test()

    @add_test_categories(["msvcstl"])
    def test_ptr_and_ref_msvcstl(self):
        self.build()
        self.do_test_ptr_and_ref()
