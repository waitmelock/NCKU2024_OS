#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x339d3416, "inode_init_owner" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0xcaa6e957, "d_instantiate" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xd976db1e, "new_inode" },
	{ 0xfa550e49, "unregister_filesystem" },
	{ 0x172dd541, "simple_statfs" },
	{ 0xe6a41b61, "d_make_root" },
	{ 0x4c236f6f, "__x86_indirect_thunk_r15" },
	{ 0xb41732ed, "d_splice_alias" },
	{ 0xea4ba114, "current_time" },
	{ 0x222a9546, "iput" },
	{ 0xb40dfe8b, "register_filesystem" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x493b1402, "simple_inode_init_ts" },
	{ 0x65487097, "__x86_indirect_thunk_rax" },
	{ 0x122c3a7e, "_printk" },
	{ 0x71bf4192, "make_kuid" },
	{ 0xa916b694, "strnlen" },
	{ 0x87a21cb3, "__ubsan_handle_out_of_bounds" },
	{ 0x917e9ceb, "set_nlink" },
	{ 0x5a921311, "strncmp" },
	{ 0x3997a409, "from_kgid" },
	{ 0x9166fada, "strncpy" },
	{ 0x7fe41fea, "default_llseek" },
	{ 0x626dac1d, "from_kuid" },
	{ 0xfb578fc5, "memset" },
	{ 0x42567b6, "__insert_inode_hash" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xc86bb1dd, "make_kgid" },
	{ 0x6add8044, "mount_nodev" },
	{ 0x999e8297, "vfree" },
	{ 0x2186d688, "generic_delete_inode" },
	{ 0x858a072d, "generic_file_open" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x64210cf6, "__mark_inode_dirty" },
	{ 0xb377bfab, "generic_file_llseek" },
	{ 0xb5b54b34, "_raw_spin_unlock" },
	{ 0xbc314156, "nop_mnt_idmap" },
	{ 0xf079b8f9, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "F96BB039C55322ADA29A19B");
