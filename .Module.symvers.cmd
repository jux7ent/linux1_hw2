cmd_/home/jux7ent/Documents/Linux1_hw2/Module.symvers := sed 's/ko$$/o/' /home/jux7ent/Documents/Linux1_hw2/modules.order | scripts/mod/modpost -m -a   -o /home/jux7ent/Documents/Linux1_hw2/Module.symvers -e -i Module.symvers   -T -