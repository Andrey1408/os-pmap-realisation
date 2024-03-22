#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/printk.h>
#include <linux/seq_file.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/mm.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("OS. Lab2. debugfs: pmap");

static struct dentry *dir;
static struct dentry *pid_file;
static struct dentry *pmap_file;

static u32 pid = -1;

static int pid_write_op(void *data, u64 value) {
    pid = value;
    pr_info("pmap: input pid is %d.\n", pid);
    return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pid_ops, NULL, pid_write_op, "%llu\n");

static void print_pmap(struct seq_file *m, struct task_struct *ts) {
    struct vm_area_struct *vma;
    unsigned long start_addr, end_addr;
    unsigned long size_kb = 0;
    unsigned long pages_count = 0;

    seq_printf(m, "Address       Kbytes      Mode     Mapping\n");

    for (vma = ts->mm->mmap; vma; vma = vma->vm_next) {
        start_addr = vma->vm_start;
        end_addr = vma->vm_end;
        size_kb = (end_addr - start_addr) / 1024;

        // Calculate RSS (Resident Set Size) in kilobytes
        pages_count = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
        size_kb = pages_count * (PAGE_SIZE / 1024);

        seq_printf(m, "%08lx-%08lx %8luK ", start_addr, end_addr, size_kb);

        // Print permission flags
        seq_printf(m, "%c%c%c", vma->vm_flags & VM_READ ? 'r' : '-',
                   vma->vm_flags & VM_WRITE ? 'w' : '-',
                   vma->vm_flags & VM_EXEC ? 'x' : '-');

        // Print mapping type
        if (vma->vm_file)
            seq_printf(m, "  file  %s\n", vma->vm_file->f_path.dentry->d_iname);
        else if (vma->vm_mm->exe_file)
            seq_printf(m, "  exe   %s\n", vma->vm_mm->exe_file->f_path.dentry->d_iname);
        else
            seq_printf(m, "  [heap]\n");
    }
}


static int show(struct seq_file *m, void *v) {
    struct task_struct *ts;

    ts = get_pid_task(find_get_pid(pid), PIDTYPE_PID);
    if (!ts) {
        pr_info("pmap_mod: task_struct with pid %d has not been found.\n", pid);
        seq_printf(m, "task_struct with pid %d has not been found.\n", pid);
        return 0;
    }

    print_pmap(m, ts);
    return 0;
}

static int open(struct inode *inode, struct file *file) {
    return single_open(file, show, NULL);
}

static const struct file_operations pmap_ops = {
    .llseek = seq_lseek,
    .open = open,
    .owner = THIS_MODULE,
    .read = seq_read,
    .release = single_release,
};

static int kmod_init(void) {
    pr_info("pmap_mod: module loading.\n");

    dir = debugfs_create_dir("pmap", NULL);
    pid_file = debugfs_create_file("pid_here", 0222, dir, NULL, &pid_ops);
    pmap_file = debugfs_create_file("pmap_output", 0622, dir, NULL, &pmap_ops);

    return 0;
}

static void kmod_exit(void) {
    debugfs_remove_recursive(dir);
    pr_info("pmap: module unloaded\n");
}

module_init(kmod_init);
module_exit(kmod_exit);



