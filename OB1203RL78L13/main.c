typedef struct st_ob1203
{
  /* Low-level operations */
  void (* reset)(void);

}ob1203_t;

void OB1203__reset(void)
{

}

ob1203_t OB1203 =
{
  .reset = OB1203__reset,
};

int main( void )
{
  return 0;
}
